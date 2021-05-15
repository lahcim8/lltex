#include "ptexlib.h"
#include <stdlib.h>

boolean doing_leaders = false;  /* are we inside a leader box? */

/* From WEB2C */

void recorder_change_filename(char *filename)
{
	(void)filename;
}
void recorder_record_input(const char *input_filename)
{
	(void)input_filename;
}
void recorder_record_output(const char *output_filename)
{
	(void)output_filename;
}

char *fullnameoffile;
int recorder_enabled;
char *output_directory;

__attribute__ ((noreturn))
void
uexit (int unix_code)
{
	int final_code;

	if (unix_code == 0)
		final_code = EXIT_SUCCESS;
	else if (unix_code == 1)
		final_code = EXIT_FAILURE;
	else
		final_code = unix_code;

	exit(final_code);
}

void
usage (const char *str)
{
	fprintf (stderr, "Try `%s --help' for more information.\n", str);
	uexit (1);
}

void
usagehelp (const char **message, const char *bug_email)
{
	if (!bug_email)
		bug_email = "tex-k@tug.org";
	while (*message) {
		printf("%s\n", *message);
		++message;
	}
	printf("\nEmail bug reports to %s.\n", bug_email);
	uexit(0);
}

void *
xmalloc(size_t size)
{
	void *alloc = malloc(size ? size : 1);
	if (!alloc) {
		fprintf(stderr, "xmalloc failed for %zu\n", size);
		exit(EXIT_FAILURE);
	}
	return alloc;
}

void *
xrealloc(void *old, size_t size)
{
	if (!old) {
		return xmalloc(size);
	}

	void *alloc = realloc(old, size ? size : 1);
	if (!alloc) {
		fprintf(stderr, "xrealloc failed for %zu\n", size);
		exit(EXIT_FAILURE);
	}
	return alloc;
}

void *
xcalloc(size_t n, size_t size)
{
	void *alloc = calloc(n ? n : 1, size ? size : 1);
	if (!alloc) {
		fprintf(stderr, "xcalloc failed for %zu\n", size);
		exit(EXIT_FAILURE);
	}
	return alloc;
}

char *
xstrdup(const char *s)
{
	char *new = xmalloc(strlen(s) + 1);
	return strcpy(new, s);
}

char *
concat3(const char *s1, const char *s2, const char *s3)
{
	int len1 = s1 ? strlen(s1) : 0;
	int len2 = s2 ? strlen(s2) : 0;
	int len3 = s3 ? strlen(s3) : 0;

	char *result = xmalloc(len1 + len2 + len3 + 1);

	if (s1) {
		strcpy(result, s1);
	} else {
		result[0] = 0;
	}
	if (s2) {
		strcat(result, s2);
	}
	if (s3) {
		strcat(result, s3);
	}
	return result;
}

char *
concat(const char *s1, const char *s2)
{
	return concat3(s1, s2, NULL);
}

