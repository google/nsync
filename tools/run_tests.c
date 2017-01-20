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

/* Run the tests listed on the command line. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(_WIN32)
#include <Windows.h>
#define popen _popen
#define pclose _pclose
#define sleep(n) Sleep ((n) * 1000)
#define mkstemp(x) (-1)
#define TEMP tmpnam (NULL)
#define closefd(fd)
#else
#include <unistd.h>
#define TEMP templ
#define closefd(fd) close (fd)
#endif

/* If x is non-zero, print a usage message on stderr, and exit
   with a non-zero status. */
static void usage_if (int x) {
        if (x) {
                fprintf (stderr,
                         "usage: run_tests [-p <N>] [-n <N>] [-bB] test...\n"
                         "  -p <N>   runs <N> tests in parallel\n"
                         "  -n <N>   runs <N> subtests in parallel\n"
                         "  -b       benchmarks are run if tests pass\n"
                         "  -B       only benchmarks are run\n"
                         "Benchmarks are not run in parallel.\n");
                exit (2);
        }
}

/* Describe a run of a test */
struct test_run {
        char *name;     /* test name (execuatble to run) */
        char *cmd;      /* command line with flags to be passed to popen() */
        char *file;     /* output file */
	int fd;		/* output file fd or -1 */

        FILE *pfp;      /* result of popen(); NULL when run is complete */
        int status;     /* result of pclose */
};

/* Return whether the file with nul-terminated name file[] can be read, and
   ends with a newline.  Also returns true if file==NULL.*/
static int file_ready (const char *file) {
        int lastc = '\n';
        if (file != NULL) {
                FILE *fp = fopen (file, "r");
                lastc = -1;
                if (fp != NULL) {
                        int c;
                        lastc = '\n';
                        while ((c = getc (fp)) != EOF) {
                                lastc = c;
                        }
                        fclose (fp);
                }
        }
        return (lastc == '\n');
}

/* If file != NULL, output the contents of file with nul-terminated name file[]
   to ofp.  Return 2 if I/O failed, and 0 otherwise.  */
static int output (FILE *ofp, const char *file) {
        int rc = 0;
        if (file != NULL) {
                FILE *fp = fopen (file, "r");
                if (fp == NULL) {
                        perror (file);
                        rc = 2;
                } else {
                        int c;
                        while ((c = getc (fp)) != EOF) {
                                putc (c, ofp);
                        }
                        if (ferror (fp)) {
                                perror (file);
                                rc = 2;
                        }
                        if (fclose (fp) != 0) {
                                perror (file);
                                rc = 2;
                        }
                }
                unlink (file);
                fflush (ofp);
        }
        return (rc);
}

/* Run the tests test[0, .., n-1], running as many as possible in parallel,
   Assumes that test[*].{name,cmd,file} are filled in, and other
   fields are uninitialized.
   On return, all fields are initialized.
   It is expected that all output has been directed to the file named by
   test[*].file[] (if parallelism > 1) or test[*].pfp (if parallelism==1).
   test[*].status is valid, and test[*].pfp == NULL.  Any output from the
   commands is written to *ofp, and test[*].file[] is removed.
   Returns 0 on success, 2 if any I/O error occurred, and 1 if a test failed,
   and 3 if a test failed and there was an I/O error.

   Attempts to run "parallelism" tests in parallel.
   The parallelism is half-hearted as we avoid threads and
   non-blocking I/O for portability.  */
static int run (FILE *ofp, int n, struct test_run *test, int parallelism) {
        int running = 0;
        int started = 0;
        int flushed = 0;
        int rc = 0;
        int i;
        while (flushed != n) {
                for (i = 0; i != started; i++) {
                        if (test[i].pfp != NULL &&
                            file_ready (test[i].file)) {
                                int c;
                                int err;
                                while ((c = getc (test[i].pfp)) != EOF) {
                                        putc (c, ofp);
                                        if (c == '\n' || c == ' ') {
                                                fflush (ofp);
                                        }
                                }
                                err = ferror (test[i].pfp);
                                test[i].status = pclose (test[i].pfp);
                                if (test[i].status != 0) {
                                        rc |= 1;
                                } else if (err) {
                                        test[i].status = 1;
                                        rc |= 1;
                                }
                                test[i].pfp = NULL;
                                running--;
                        }
                }
                while (flushed != started && test[flushed].pfp == NULL) {
                        rc |= output (ofp, test[flushed].file);
                        flushed++;
                }
                while (running < parallelism && started != n) {
                        test[started].pfp = popen (test[started].cmd, "r");
                        test[started].status = 2;
                        running += (test[started].pfp != NULL);
                        started++;
                }
                if (parallelism > 1) {
                        sleep (1);
                }
        }
        if ((rc & 1) != 0) {
                fprintf (stderr, "failures:");
                for (i = 0; i != n; i++) {
                        if (test[i].status != 0) {
                                fprintf (stderr, " %s\n", test[i].name);
                        }
                }
                fprintf (stderr, "\n");
        }
        return (rc);
}

/* For each test run test[0, .., n-1], test run, free its output file name and
   cmd string.  */
static void cleanup (int n, struct test_run *test) {
        int i;
        for (i = 0; i != n; i++) {
		if (test[i].fd != -1) {
			closefd (test[i].fd);
		}
                if (test[i].file != NULL) {
                        free (test[i].file);
                }
                free (test[i].cmd);
        }
}

/* Run the tests in test[0, .., n-1] with the deisgnated parallism.
   Run Benchmarks instead of tests if is_benchmark != 0.
   On entry, only the test[*].name field is valid.
   Return a suitable exit code, 0 for success, and non-zero for failure.
   Output is written to *ofp. */
static int run_tests (FILE *ofp, int n, struct test_run *test,
              int parallelism, int subparallelism, int is_benchmark) {
        int i;
        int rc;
        for (i = 0; i != n; i++) {
                int cmd_len;
                char flags[64];
                char *redirect;
                if (parallelism > 1) {
                        int redirect_len;
			char templ[64];
			snprintf (templ, sizeof (templ),
				  "/tmp/run_tests.XXXXXXXXX");
			test[i].fd = mkstemp (templ);
                        test[i].file = strdup (TEMP);
                        redirect_len = strlen (test[i].file) + 10;
                        redirect = (char *) malloc (redirect_len);
                        snprintf (redirect, redirect_len, "> \"%s\"",
                                  test[i].file);
                } else {
                        test[i].file = NULL;
                        test[i].fd = -1;
                        redirect = strdup ("");
                }
                if (subparallelism == 1) {
                        snprintf (flags, sizeof (flags), "%s",
                                  is_benchmark? "-B" : "");
                } else {
                        snprintf (flags, sizeof (flags), "%s -n %d",
                                  is_benchmark? "-B" : "",
                                  subparallelism);
                } 
                cmd_len = strlen (test[i].name) +
                          strlen (redirect) + strlen (flags) + 64;
                test[i].cmd = (char *) malloc (cmd_len);
                snprintf (test[i].cmd, cmd_len, "%s %s %s 2>&1",
                                  test[i].name, flags, redirect);
                free (redirect);
        }
        rc = run (ofp, n, test, parallelism);
        cleanup (n, test);
        return (rc);
}

int main (int argc, char *argv[]) {
        int do_run_benchmarks = 0;
        int do_run_tests = 1;
        int parallelism = 1;
        int subparallelism = 1;
        int test_n = 0;
        int test_max = 16;
        struct test_run *test =
                (struct test_run *) malloc (sizeof (test[0]) * test_max);
        int rc = 0;
        int argn;

        for (argn = 1; argn != argc; argn++) {
                const char *arg = argv[argn];
                if (arg[0] == '-') {
                        const char *f;
                        for (f = &arg[1]; *f != 0; f++) {
                                switch (*f) {
                                case 'p': usage_if (argn+1 == argc);
                                          parallelism = atoi (argv[++argn]);
                                          break;
                                case 'n': usage_if (argn+1 == argc);
                                          subparallelism = atoi (argv[++argn]);
                                          break;
                                case 'b': do_run_benchmarks = 1;
                                          break;
                                case 'B': do_run_benchmarks = 1;
                                          do_run_tests = 0;
                                          break;
                                default:  usage_if (1);
                                }
                        }
                } else {
                        if (test_n == test_max) {
                                test_max *= 2;
                                test = (struct test_run *) realloc (    
                                        test, sizeof (test[0]) * test_max);
                        }
                        test[test_n++].name = argv[argn];
                }
        }

        if (do_run_tests) {
                rc |= run_tests (stdout, test_n, test,
                                 parallelism, subparallelism, 0);
        }
        if (do_run_benchmarks && rc == 0) {
                rc |= run_tests (stdout, test_n, test, 1, 1, 1);
        }

        free (test);
        return (rc);
}
