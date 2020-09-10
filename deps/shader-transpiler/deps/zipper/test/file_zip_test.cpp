#include "catch.hpp"

#include <zipper/zipper.h>
#include <zipper/unzipper.h>
#include <zipper/tools.h>

#include <vector>
#include <fstream>
#include <ostream>
#include <string>
#include <map>

using namespace zipper;

SCENARIO("zipfile feed with different inputs", "[zip]")
{
    GIVEN("A Zip outputed to a file")
    {
        // just in case the last test fails there
        // will still be one zip file around delete it first

        if (checkFileExists("ziptest.zip"))
            std::remove("ziptest.zip");

        zipper::Zipper zipper("ziptest.zip");

        WHEN("a file containing 'test file compression' is added and named 'test1'")
        {
            std::ofstream test1("test1.txt");
            test1 << "test file compression";
            test1.flush();
            test1.close();

            std::ifstream test1stream("test1.txt");

            zipper.add(test1stream, "test1.txt");

            test1stream.close();
            zipper.close();

            std::remove("test1.txt");

            zipper::Unzipper unzipper("ziptest.zip");

            THEN("the zip file has one entry named 'test1.txt'")
            {
                std::vector<zipper::ZipEntry> entries = unzipper.entries();
                REQUIRE(entries.size() == 1);
                REQUIRE(entries.front().name == "test1.txt");

                AND_THEN("extracting the test1.txt entry creates a file named 'test1.txt' with the text 'test file compression'")
                {
                    unzipper.extractEntry("test1.txt");
                    // due to sections forking or creating different stacks we need to make sure the local instance is closed to
                    // prevent mixing the closing when both instances are freed at the end of the scope
                    unzipper.close();

                    REQUIRE(checkFileExists("test1.txt"));

                    std::ifstream testfile("test1.txt");
                    REQUIRE(testfile.good());

                    std::string test((std::istreambuf_iterator<char>(testfile)), std::istreambuf_iterator<char>());
                    testfile.close();
                    REQUIRE(test == "test file compression");

                    AND_WHEN("another file containing 'other data to compression test' and named 'test2.dat' is added inside a folder 'TestFolder'")
                    {
                        std::ofstream test2("test2.dat");
                        test2 << "other data to compression test";
                        test2.flush();
                        test2.close();

                        std::ifstream test2stream("test2.dat");

                        zipper.open();
                        zipper.add(test2stream, "TestFolder/test2.dat");
                        zipper.close();

                        test2stream.close();
                        std::remove("test2.dat");

                        zipper::Unzipper unzipper("ziptest.zip");

                        AND_THEN("the zip file has two entrys named 'test1.txt' and 'TestFolder/test2.dat'")
                        {
                            REQUIRE(unzipper.entries().size() == 2);
                            REQUIRE(unzipper.entries().front().name == "test1.txt");
                            REQUIRE(unzipper.entries()[1].name == "TestFolder/test2.dat");

                            AND_THEN("extracting the test2.dat entry creates a folder 'TestFolder' with a file named 'test2.dat' with the text 'other data to compression test'")
                            {
                                unzipper.extract();
                                unzipper.close();

                                REQUIRE(checkFileExists("TestFolder/test2.dat"));

                                std::ifstream testfile("TestFolder/test2.dat");
                                REQUIRE(testfile.good());

                                std::string test((std::istreambuf_iterator<char>(testfile)), std::istreambuf_iterator<char>());
                                testfile.close();
                                REQUIRE(test == "other data to compression test");

                                AND_WHEN("adding a folder to the zip, creates one entry for each file inside the folder with the name in zip as 'Folder/...'")
                                {
                                    makedir(currentPath() + "/TestFiles/subfolder");
                                    std::ofstream test("TestFiles/test1.txt");
                                    test << "test file compression";
                                    test.flush();
                                    test.close();

                                    std::ofstream test1("TestFiles/test2.pdf");
                                    test1 << "test file compression";
                                    test1.flush();
                                    test1.close();

                                    std::ofstream test2("TestFiles/subfolder/test-sub.txt");
                                    test2 << "test file compression";
                                    test2.flush();
                                    test2.close();

                                    zipper.open();
                                    zipper.add("TestFiles");
                                    zipper.close();

                                    zipper::Unzipper unzipper("ziptest.zip");
                                    REQUIRE(unzipper.entries().size() == 5);

                                    AND_THEN("extracting to a new folder 'NewDestination' creates the file structure from zip in the new destination folder")
                                    {
                                        makedir(currentPath() + "/NewDestination");

                                        unzipper.extract(currentPath() + "/NewDestination");

                                        std::vector<std::string> files =  zipper::filesFromDirectory(currentPath() + "/NewDestination");

                                        REQUIRE(checkFileExists("NewDestination/TestFiles/test1.txt"));
                                        REQUIRE(checkFileExists("NewDestination/TestFiles/test2.pdf"));
                                        REQUIRE(checkFileExists("NewDestination/TestFiles/subfolder/test-sub.txt"));
                                    }

                                    unzipper.close();
                                }
                            }
                        }

                        removeFolder("TestFolder");
                        removeFolder("TestFiles");
                        removeFolder("NewDestination");
                        std::remove("test1.txt");
                    }
                }
            }

            std::remove("ziptest.zip");
        }

        WHEN("a stringstream containing 'test string data compression' is added and named 'strdata'")
        {
            std::stringstream strdata;
            strdata << "test string data compression";

            zipper.add(strdata, "strdata");
            zipper.close();

            zipper::Unzipper unzipper("ziptest.zip");

            THEN("the zip file has one entry named 'strdata'")
            {
                REQUIRE(unzipper.entries().size() == 1);
                REQUIRE(unzipper.entries().front().name == "strdata");

                AND_THEN("extracting the strdata entry creates a file named 'strdata' with the txt 'test string data compression'")
                {
                    unzipper.extract();

                    REQUIRE(checkFileExists("strdata"));

                    std::ifstream testfile("strdata");
                    REQUIRE(testfile.good());

                    std::string test((std::istreambuf_iterator<char>(testfile)), std::istreambuf_iterator<char>());
                    testfile.close();
                    REQUIRE(test == "test string data compression");

                    AND_THEN("extracting with an alternative name 'alternative_strdata.dat' crates a file with that name instead of the one inside de zip")
                    {
                        std::map<std::string, std::string> alt_names;
                        alt_names["strdata"] = "alternative_strdata.dat";

                        unzipper.extract("", alt_names);

                        REQUIRE(checkFileExists("alternative_strdata.dat"));

                        std::ifstream testfile2("alternative_strdata.dat");
                        REQUIRE(testfile2.good());

                        std::string test2((std::istreambuf_iterator<char>(testfile2)), std::istreambuf_iterator<char>());
                        testfile2.close();
                        REQUIRE(test2 == "test string data compression");

                        AND_THEN("trying to extract a file 'fake.dat' that doesn't exists, returns false")
                        {
                            REQUIRE(false == unzipper.extractEntry("fake.dat"));
                        }
                    }

                    unzipper.close();
                }
            }

            std::remove("strdata");
            std::remove("alternative_strdata.dat");
            std::remove("ziptest.zip");
        }
    }
}
