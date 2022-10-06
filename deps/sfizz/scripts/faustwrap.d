#!/usr/bin/env rdmd

import std.conv;
import std.algorithm;
import std.getopt;
import std.process;
import std.regex;
import std.uni;
import std.ascii;
import std.string;
import std.array;
import std.stdio;
import core.stdc.stdlib;

enum string prologue = `#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
`;

enum string epilogue = `
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#undef FAUSTFLOAT
#undef FAUSTCLASS
`;

void main(string[] args)
{
    struct Opts {
        string outputPath;
        string className = "mydsp";
        string superclassName = null;
        bool oneSample = false;
        bool doublePrecision = false;
        bool inPlace = false;
        bool vectorize = false;
        bool mathApproximation = false;
        string processName = null;
        string[] importDirs;
    }

    Opts opts;

    auto optInfo = getopt(args,
        "out|o", "Output path", &opts.outputPath,
        "cn", "Class name", &opts.className,
        "scn", "Superclass name", &opts.superclassName,
        "os", "One sample", &opts.oneSample,
        "double", "Double precision", &opts.doublePrecision,
        "inpl", "In-place", &opts.inPlace,
        "vec", "Vectorization", &opts.vectorize,
        "mapp", "Math approximation", &opts.mathApproximation,
        "pn", "Process name", &opts.processName,
        "import-dir|I", "Import directory", &opts.importDirs);

    if (optInfo.helpWanted)
    {
        defaultGetoptPrinter("MyFaust", optInfo.options);
        return;
    }

    if (args.length != 2)
    {
        writeln("You must indicate exactly 1 input file.");
        exit(1);
    }

    string[] cmd = [getFaust()];
    cmd ~= "-cn";
    cmd ~= opts.className;
    if (opts.superclassName)
    {
        cmd ~= "-scn";
        cmd ~= opts.superclassName;
    }
    if (opts.oneSample)
        cmd ~= "-os";
    if (opts.doublePrecision)
        cmd ~= "-double";
    if (opts.inPlace)
        cmd ~= "-inpl";
    if (opts.vectorize)
        cmd ~= "-vec";
    if (opts.mathApproximation)
        cmd ~= "-mapp";
    if (opts.processName)
    {
        cmd ~= "-pn";
        cmd ~= opts.processName;
    }
    foreach (string dir; opts.importDirs)
    {
        cmd ~= "-I";
        cmd ~= dir;
    }
    cmd ~= args[1];

    auto result = execute(cmd, null, Config.stderrPassThrough|Config.suppressConsole);
    if (result.status != 0)
        exit(1);

    string code = result.output;
    Parameter[] params = findParameters(code);

    code = removeVirtualKeyword(code);
    foreach (string method; ["metadata", "getInputRate", "getOutputRate", "clone", "buildUserInterface"])
        code = removeMethod(code, method);
    foreach (string method; ["getNumInputs", "getNumOutputs"])
        code = makeMethodStaticConstexpr(code, method);
    if (!opts.superclassName)
        code = removeSuperclass(code);
    code = makePointerArgsConst(code, opts.oneSample);
    code = addParameters(code, params);

    foreach (string method; ["compute", "classInit", "instanceConstants", "instanceResetUserInterface", "instanceClear", "init", "instanceInit"])
        code = addMethodStartEnd(code, method);
    code = addClassStartEnd(code);

    File outFile = stdout;
    if (opts.outputPath)
        outFile = File(opts.outputPath, "w");
    outFile.writef("%s%s%s", prologue, code, epilogue);
    outFile.flush();
}

final class Parameter
{
    string name;
    string var;
    bool readonly;
};

Parameter[] findParameters(string code)
{
    Parameter[] params;

    auto expr = regex(`->add(Button|CheckButton|VerticalSlider|HorizontalSlider|NumEntry|HorizontalBargraph|VerticalBargraph)\("([^"]*)", &([a-zA-Z0-9_]+)`);

    foreach (string line; code.lineSplitter)
    {
        auto match = line.matchFirst(expr);
        if (match)
        {
            Parameter param = new Parameter;
            param.name = match[2];
            param.var = match[3];
            param.readonly = ["HorizontalBargraph", "VerticalBargraph"].canFind(match[1]);
            params ~= param;
        }
    }

    return params;
}

string removeVirtualKeyword(string code)
{
    auto expr = regex(`\bvirtual\s*\b`);
    return code.replaceAll(expr, "");
}

string removeMethod(string code, string method)
{
    string[] newLines;
    newLines.reserve(1024);

    auto expr = regex(`^(\s*).*\b` ~ method.escaper.to!string ~ `\s*\(.*\{\s*$`);

    bool inMethod = false;
    string eom = null;

    foreach (string line; code.lineSplitter)
    {
        if (inMethod)
        {
            if (line.stripRight == eom)
                inMethod = false;
        }
        else
        {
            auto match = line.matchFirst(expr);
            if (match)
            {
                inMethod = true;
                eom = match[1] ~ '}';
            }
            else
                newLines ~= line;
        }
    }

    return newLines.join('\n');
}

string makeMethodStaticConstexpr(string code, string method)
{
    string[] newLines;
    newLines.reserve(1024);

    auto expr = regex(`^(\s*)(.*\b` ~ method.escaper.to!string ~ `\s*\(.*\{\s*)$`);

    foreach (string line; code.lineSplitter)
    {
        auto match = line.matchFirst(expr);
        if (match)
            newLines ~= match[1] ~ "static constexpr " ~ match[2];
        else
            newLines ~= line;
    }

    return newLines.join('\n');
}

string removeSuperclass(string code)
{
    auto expr = regex(`\s*:\s*public\s+dsp\s*`);
    return code.replaceFirst(expr, " ");
}

string makePointerArgsConst(string code, bool oneSample)
{
    if (!oneSample)
    {
        auto expr = regex(`\bvoid\s+compute\s*\(\s*([^,)]+)\s*,\s*([^,)]+)\s*,\s*([^,)]+)\s*\)`);
        auto match = code.matchFirst(expr);

        string arg1 = match[1];
        string arg2 = match[2];
        string arg3 = match[3];

        auto exprArg = regex(`^FAUSTFLOAT\s*\*\s*\*`);
        arg2 = arg2.replaceFirst(exprArg, "FAUSTFLOAT const* const*");
        arg3 = arg3.replaceFirst(exprArg, "FAUSTFLOAT* const*");

        ulong start = match[0].ptr - code.ptr;
        ulong end = start + match[0].length;
        code = format("%svoid compute(%s, %s, %s)%s", code[0..start], arg1, arg2, arg3, code[end..$]);

        //
        auto exprStmt = regex(`FAUSTFLOAT\s*\*\s*(input\d+)`);
        code = code.replaceAll(exprStmt, "FAUSTFLOAT const* $1");
    }
    else
    {
        auto expr = regex(`\bvoid\s+compute\s*\(\s*([^,)]+)\s*,\s*([^,)]+)\s*,\s*([^,)]+)\s*,\s*([^,)]+)\s*\)`);
        auto match = code.matchFirst(expr);

        string arg1 = match[1];
        string arg2 = match[2];
        string arg3 = match[3];
        string arg4 = match[4];

        auto exprArg = regex(`^(FAUSTFLOAT|int)\s*\*`);
        arg1 = arg1.replaceFirst(exprArg, "$1 const*");
        arg3 = arg3.replaceFirst(exprArg, "$1 const*");
        arg4 = arg4.replaceFirst(exprArg, "$1 const*");

        ulong start = match[0].ptr - code.ptr;
        ulong end = start + match[0].length;
        code = format("%svoid compute(%s, %s, %s, %s)%s", code[0..start], arg1, arg2, arg3, arg4, code[end..$]);
    }

    return code;
}

string addParameters(string code, Parameter[] params)
{
    string[] addedLines;

    foreach (Parameter param; params)
    {
        string camelName = param.name.camelify;
        addedLines ~= "";
        addedLines ~= format(`    FAUSTFLOAT get%s() const { return %s; }`, camelName, param.var);
        if (!param.readonly)
            addedLines ~= format(`    void set%s(FAUSTFLOAT value) { %s = value; }`, camelName, param.var);
    }

    return addToClass(code, addedLines.join('\n'));
}

string addMethodStartEnd(string code, string method)
{
    string[] newLines;
    newLines.reserve(1024);

    auto expr = regex(`^(\s*).*\b` ~ method.escaper.to!string ~ `\s*\(.*\{\s*$`);

    bool inMethod = false;
    string eom = null;

    foreach (string line; code.lineSplitter)
    {
        if (inMethod)
        {
            if (line.stripRight == eom)
            {
                newLines ~= "\t\t//[End:" ~ method ~ "]";
                inMethod = false;
            }
            newLines ~= line;
        }
        else
        {
            newLines ~= line;
            auto match = line.matchFirst(expr);
            if (match)
            {
                inMethod = true;
                eom = match[1] ~ '}';
                newLines ~= "\t\t//[Begin:" ~ method ~ "]";
            }
        }
    }

    return newLines.join('\n');
}

string addClassStartEnd(string code)
{
    {
        auto expr = regex(`^class\b.*$`, "m");
        auto match = code.matchFirst(expr);
        ulong start = match[0].ptr - code.ptr;
        ulong end = start + match[0].length;
        code = code[0..start] ~ "\n//[Before:class]\n" ~ match[0] ~ "\n\t//[Begin:class]\n" ~ code[end..$];
    }

    {
        auto expr = regex(`^\};`, "m");
        auto match = code.matchLast(expr);
        ulong start = match[0].ptr - code.ptr;
        ulong end = start + match[0].length;
        code = code[0..start] ~ "\n\t//[End:class]\n" ~ match[0] ~ "\n//[After:class]\n" ~ code[end..$];
    }

    return code;
}

string addToClass(string code, string addend)
{
    if (addend.empty)
        return code;

    auto expr = regex(`^\};`, "m");
    auto match = code.matchLast(expr);
    ulong index = match[0].ptr - code.ptr;
    return code[0..index] ~ addend ~ '\n' ~ code[index..$];
}

string camelify(string name)
{
    dchar[] result;
    result.reserve(name.length);

    bool isIdentifierChar(dchar ch)
    {
        return std.ascii.isAlphaNum(ch) || ch == '_';
    }

    dchar[] temp = name.to!(dchar[]);

    foreach (ref dchar uniChar; temp)
    {
        uniChar = [uniChar].normalize!NFD[0];
        if (!isIdentifierChar(uniChar))
            uniChar = ' ';
    }

    foreach (dchar[] part; temp.split(' '))
    {
        if (!part.empty)
            part[0] = std.uni.toUpper(part[0]);
        result ~= part;
    }

    return result.to!string;
}

Captures!S matchLast(S, R)(S input, R expr)
{
    Captures!S last;
    foreach (Captures!S current; input.matchAll(expr))
        last = current;
    return last;
}

string getFaust()
{
    char *env = getenv("FAUST");
    return env ? env.to!string : "faust";
}
