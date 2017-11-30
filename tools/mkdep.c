/* Copyright 2016 Google Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License. */

/* Generate make dependencies using C preprocessor.
   Usage:
       mkdep cc cc_args_to_invoke_preprocessor... file.{[csh],cc} ...
   Output is written to stdout. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#if defined(_WIN32)
#define popen _popen
#define pclose _pclose
#endif

/* Comparison routine to sort strings with qsort(). */
static int cmpstr (const void *va, const void *vb) {
        const char **a = (const char **)va;
        const char **b = (const char **)vb;
        return (strcmp (*a, *b));
}

/* Return whether nul-terminated string s[] starts with any of the
   nul-terminated strings given as arguments after s,
   and termated with a NULL argument. */
static int has_prefix (const char *s, ...) {
        va_list ap;
        const char *pref;
        va_start (ap, s);
        pref = va_arg (ap, const char *);
        while (pref != NULL && strncmp (s, pref, strlen (pref)) != 0) {
                pref = va_arg (ap, const char *);
        }
        va_end (ap);
        return (pref != NULL);
}

/* Return whether nul-terminated string s[] ends with any of the
   nul-terminated strings given as arguments after s,
   and termated with a NULL argument. */
static int has_suffix (const char *s, ...) {
        va_list ap;
        int slen = strlen (s);
        const char *suf;
        va_start (ap, s);
        suf = va_arg (ap, const char *);
        while (suf != NULL &&
               (slen < (int) strlen (suf) ||
                strcmp (&s[slen-strlen (suf)], suf) != 0)) {
                suf = va_arg (ap, const char *);
        }
        va_end (ap);
        return (suf != NULL);
}

/* Return whether nul-termainted string s[] ends with
   a suffix that designates a source file. */
static int has_prog_suffix (const char *s) {
        return (has_suffix (s, ".c", ".h", ".s", ".cc",
                            ".C", ".H", ".S", ".CC",
                            (char *) NULL));
}

/* Return the index in nul-terminated string s[] of the longest suffix that
   contains none of the bytes in nul-terminated string sep[].  */
static int pos_suffix_not_containing (const char *s, const char *sep) {
        int pos = strlen (s);
        while (pos != 0 && strchr (sep, s[pos-1] & 0xff) == NULL) {
                pos--;
        }
        return (pos);
}

/* Process a source file with nul-terminated file name file[],
   writing to ofp the source dependencies obtained by
   running the file through the C preprocessor using the
   nul-terminated command prefix cmd_prefix[]. */
static void process_file (FILE *ofp, const char *cmd_prefix, const char *file) {
        char line[2048];
        int cmdlen = strlen (cmd_prefix) + strlen (file) + 10;
        char *cmd = (char *) malloc (cmdlen);
        FILE *ifp;
        int status;
        int i;
        int n;
        int basename = pos_suffix_not_containing (file, "/\\");
        int ext = pos_suffix_not_containing (file, ".");
        const char *last;
        int list_max = 64;
        char **list = (char **) malloc (sizeof (list[0]) * list_max);

        snprintf (cmd, cmdlen, "%s %s", cmd_prefix, file);
        ifp = popen (cmd, "r");
        if (ifp == NULL) {
                fprintf (stderr, "error starting command: \"%s\": ", cmd);
                perror ("popen");
                exit (2);
        }
        n = 0;
        while (fgets (line, sizeof (line), ifp) != NULL) {
                if (has_prefix (line, "#line 1 \"", "# 1 \"", (char *) NULL)) {
                        char *start = strchr (line, '"');
                        char *end = strchr (start+1, '"');
                        if (end != NULL)  {
                                end[1] = 0;
                                if (start[0] == '"' && end[0] == '"' &&
                                    strchr (start, ' ') == NULL) {
                                        /* No need for quotes. */
                                        start++;
                                        end[0] = 0;
                                }
                        }
                        if (has_prog_suffix (start)) {
                                if (n == list_max) {
                                        list_max *= 2;
                                        list = (char **) realloc (list,
                                                sizeof (list[0]) * list_max);
                                }
                                list[n++] = strdup (start);
                        }
                }
        }
        if (basename < ext) {
                fprintf (ofp, "%.*so:", ext - basename, &file[basename]);
        } else {
                abort (); /* can't happen: file ends in a prog suffix */
        }
        qsort (list, n, sizeof (list[0]), &cmpstr);
        last = "";
        for (i = 0; i != n; i++) {
                if (strcmp (last, list[i]) != 0) {
                        fprintf (ofp, " \\\n %s", list[i]);
                        last = list[i];
                }
        }
        fprintf (ofp, "\n");
        for (i = 0; i != n; i++) {
                free (list[i]);
        }
        if (ferror (ifp)) {
                fprintf (stderr, "error reading from command: ");
                perror (cmd);
                exit (2);
        }
        status = pclose (ifp);
        if (status != 0) {
                fprintf (stderr,
                         "mkdep: command \"%s\" returned error %x\n",
                         cmd, status);
                exit (2);
        }
        free (cmd);
        free (list);
}

int main (int argc, char *argv[]) {
        char cmd_prefix[2048];
        int cmd_len = 0;
        char cpp_cmd_prefix[2048];
        int cpp_cmd_len = 0;
        int argn;
        FILE *ofp = stdout;
        /* Build up command line using arguments
           that don't end in .[chs] .cc */
        for (argn = 1; argn != argc; argn++) {
                const char *arg = argv[argn];
                if (!has_prog_suffix (arg)) {
                        if (has_prefix (arg, "-c++=", NULL)) {
                                snprintf (&cpp_cmd_prefix[cpp_cmd_len],
                                          sizeof (cpp_cmd_prefix) - cpp_cmd_len,
                                          "%s ", arg+5);
                                cpp_cmd_len += strlen (
                                        &cpp_cmd_prefix[cpp_cmd_len]);
                        } else {
                                snprintf (&cmd_prefix[cmd_len],
                                          sizeof (cmd_prefix) - cmd_len,
                                          "%s ", arg);
                                cmd_len += strlen (&cmd_prefix[cmd_len]);
                                snprintf (&cpp_cmd_prefix[cpp_cmd_len],
                                          sizeof (cpp_cmd_prefix) - cpp_cmd_len,
                                          "%s ", arg);
                                cpp_cmd_len += strlen (
                                        &cpp_cmd_prefix[cpp_cmd_len]);
                        }
                }
        }
        if (cmd_len >= (int) sizeof (cmd_prefix) - 1) {
                fprintf (stderr, "mkdep: command line is too long\n");
                exit (2);
        }
        if (cpp_cmd_len >= (int) sizeof (cpp_cmd_prefix) - 1) {
                fprintf (stderr, "mkdep: command line is too long\n");
                exit (2);
        }
        cmd_prefix[cmd_len] = 0;
        cpp_cmd_prefix[cpp_cmd_len] = 0;

        /* Run the compiler for each non-flag file containing a dot,
           generating Makefile dependencies for that file by running
           it through the preprocessor. */
        for (argn = 1; argn != argc; argn++) {
                if (has_prog_suffix (argv[argn])) {
                        if (has_suffix (argv[argn], ".cc", ".CC", NULL)) {
                                process_file (ofp, cpp_cmd_prefix, argv[argn]);
                        } else {
                                process_file (ofp, cmd_prefix, argv[argn]);
                        }
                }
        }
        return (0);
}
