/*
 * json.c
 *
 *  Created on: 20.01.2017
 *      Author: hoene
 */

#include "json.h"
#include "../hrtf/tools.h"
#include <stdio.h>
#include <string.h>

static void printString(FILE *out, char *string) {
  fprintf(out, "\"");
  if (string) {
    while (*string) {
      switch (*string) {
      case '"':
        fprintf(out, "\\\"");
        break;
      case '\\':
        fprintf(out, "\\\\");
        break;
      case '/':
        fprintf(out, "\\/");
        break;
      case '\b':
        fprintf(out, "\\b");
        break;
      case '\f':
        fprintf(out, "\\f");
        break;
      case '\n':
        fprintf(out, "\\n");
        break;
      case '\r':
        fprintf(out, "\\r");
        break;
      case '\t':
        fprintf(out, "\\t");
        break;
      default:
        fprintf(out, "%c", *string);
      }
      string++;
    }
  }
  fprintf(out, "\"");
}

static int printAttributes(FILE *out, int spaces, struct MYSOFA_ATTRIBUTE *attr,
                           int sanitize) {
  int i = 0;
  // search a relevant attribute)
  for (struct MYSOFA_ATTRIBUTE *n = attr; n; n = n->next) {
    if (!sanitize || (strcmp(n->name, "_NCProperties") &&
                      strcmp(n->name, "_Netcdf4Coordinates"))) {
      i++;
      break;
    }
  }
  if (i == 0)
    return 0;

  for (i = 0; i < spaces; i++)
    fprintf(out, " ");
  fprintf(out, "\"Attributes\": {\n");

  int count = 0;
  while (attr) {
    if (!sanitize || (strcmp(attr->name, "_NCProperties") &&
                      strcmp(attr->name, "_Netcdf4Coordinates"))) {
      if (count)
        fprintf(out, ",\n");
      count++;
      for (i = 0; i <= spaces; i++)
        fprintf(out, " ");
      printString(out, attr->name);
      fprintf(out, ": ");
      printString(out, attr->value);
    }
    attr = attr->next;
  }
  fprintf(out, "\n");
  for (i = 0; i < spaces; i++)
    fprintf(out, " ");
  fprintf(out, "}");
  return count;
}

/*
 "Dimensions":[
 1,
 2
 ],
 "DimensionNames":[
 "I",
 "R"
 ],
 */

static void printDimensions(FILE *out, struct MYSOFA_HRTF *hrtf,
                            struct MYSOFA_ATTRIBUTE **p) {
  struct MYSOFA_ATTRIBUTE *found = NULL;
  char *s;
  int dimensions[4];
  int dims = 0, i;

  while (*p) {
    if (!strcmp((*p)->name, "DIMENSION_LIST")) {
      found = *p;
      *p = (*p)->next;
      break;
    }
    p = &((*p)->next);
  }
  if (found) {

    fprintf(out, "   \"DimensionNames\":[");
    s = found->value;
    while (s && s[0] && dims < 4) {
      switch (s[0]) {
      case 'I':
        dimensions[dims++] = hrtf->I;
        break;
      case 'C':
        dimensions[dims++] = hrtf->C;
        break;
      case 'R':
        dimensions[dims++] = hrtf->R;
        break;
      case 'E':
        dimensions[dims++] = hrtf->E;
        break;
      case 'N':
        dimensions[dims++] = hrtf->N;
        break;
      case 'M':
        dimensions[dims++] = hrtf->M;
        break;
      }
      if (s[1] == ',') {
        fprintf(out, "\"%c\",", s[0]);
        s += 2;
      } else {
        fprintf(out, "\"%c\"", s[0]);
        break;
      }
    }
    fprintf(out, "],\n");

    fprintf(out, "   \"Dimensions\":[");
    for (i = 0; i < dims; i++) {
      if (i + 1 < dims)
        fprintf(out, "%d,", dimensions[i]);
      else
        fprintf(out, "%d],\n", dimensions[i]);
    }

    free(found->name);
    free(found->value);
    free(found);
  }
}

static int printArray(FILE *out, struct MYSOFA_HRTF *hrtf,
                      struct MYSOFA_ARRAY *array, char *name, int sanitize) {
  int i = 0;

  if (!array->elements)
    return 0;

  fprintf(out, "  ");
  printString(out, name);
  fprintf(out, ": {\n");

  fprintf(out, "   \"TypeName\":\"double\",\n");

  printDimensions(out, hrtf, &array->attributes);

  if (printAttributes(out, 3, array->attributes, sanitize))
    fprintf(out, ",\n");

  fprintf(out, "   \"Values\": [");
  for (i = 0; i < array->elements; i++) {
    fprintf(out, "%c%s%12e", i == 0 ? ' ' : ',', i % 20 == 19 ? "\n    " : "",
            array->values[i]);
  }

  fprintf(out, " ]\n  }");

  return 1;
}

/*
 * The HRTF structure data types
 */
void printJson(FILE *out, struct MYSOFA_HRTF *hrtf, int sanitize) {
  fprintf(out, "{\n");

  if (printAttributes(out, 1, hrtf->attributes, sanitize))
    fprintf(out, ",\n");

  fprintf(out, " \"Dimensions\": {\n");
  fprintf(out, "  \"I\": %d,\n", hrtf->I);
  fprintf(out, "  \"C\": %d,\n", hrtf->C);
  fprintf(out, "  \"R\": %d,\n", hrtf->R);
  fprintf(out, "  \"E\": %d,\n", hrtf->E);
  fprintf(out, "  \"N\": %d,\n", hrtf->N);
  fprintf(out, "  \"M\": %d\n", hrtf->M);
  fprintf(out, " },\n");

  fprintf(out, " \"Variables\": {\n");
  if (printArray(out, hrtf, &hrtf->ListenerPosition, "ListenerPosition",
                 sanitize))
    fprintf(out, ",\n");
  if (printArray(out, hrtf, &hrtf->ReceiverPosition, "ReceiverPosition",
                 sanitize))
    fprintf(out, ",\n");
  if (printArray(out, hrtf, &hrtf->SourcePosition, "SourcePosition", sanitize))
    fprintf(out, ",\n");
  if (printArray(out, hrtf, &hrtf->EmitterPosition, "EmitterPosition",
                 sanitize))
    fprintf(out, ",\n");
  if (printArray(out, hrtf, &hrtf->ListenerUp, "ListenerUp", sanitize))
    fprintf(out, ",\n");
  if (printArray(out, hrtf, &hrtf->ListenerView, "ListenerView", sanitize))
    fprintf(out, ",\n");
  if (printArray(out, hrtf, &hrtf->DataIR, "Data.IR", sanitize))
    fprintf(out, ",\n");
  if (printArray(out, hrtf, &hrtf->DataSamplingRate, "Data.SamplingRate",
                 sanitize))
    fprintf(out, ",\n");
  printArray(out, hrtf, &hrtf->DataDelay, "Data.Delay", sanitize);

  struct MYSOFA_VARIABLE *node = hrtf->variables;
  while (node) {
    fprintf(out, ",\n");
    printArray(out, hrtf, hrtf->variables->value, hrtf->variables->name,
               sanitize);
    node = node->next;
  }
  fprintf(out, " }\n}\n");
}
