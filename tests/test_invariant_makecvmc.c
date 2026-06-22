#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

START_TEST(test_ihx_buffer_overflow)
{
    /* Invariant: processing IHX files with oversized data fields must not
       cause out-of-bounds buffer access (buffer is 256 bytes, data field
       can be up to 512 hex chars for count=255) */
    const char *payloads[] = {
        /* Exact exploit: count=FF (255 bytes) = 512 hex chars of data */
        ":FF0000" "00" "AABBCCDD" "AABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDD"
                       "AABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDD"
                       "AABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDD"
                       "AABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDD"
                       "AABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDD"
                       "AABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDD"
                       "AABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDD"
                       "AABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDD" "00\n"
        ":00000001FF\n",
        /* Boundary: count=80 (128 bytes) = 256 hex chars, exactly fills buffer */
        ":800000" "00" "AABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDD"
                       "AABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDD"
                       "AABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDD"
                       "AABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDDAABBCCDD" "00\n"
        ":00000001FF\n",
        /* Valid: count=10 (16 bytes), normal IHX record */
        ":10010000214601360121470136007EFE09D2010024\n"
        ":00000001FF\n",
    };
    int num_payloads = sizeof(payloads) / sizeof(payloads[0]);

    for (int i = 0; i < num_payloads; i++) {
        char tmpfile[] = "/tmp/test_ihx_XXXXXX";
        int fd = mkstemp(tmpfile);
        ck_assert_int_ge(fd, 0);
        write(fd, payloads[i], strlen(payloads[i]));
        close(fd);

        char outfile[] = "/tmp/test_out_XXXXXX";
        int ofd = mkstemp(outfile);
        ck_assert_int_ge(ofd, 0);
        close(ofd);

        /* Run makecvmc in a subprocess; if it crashes (SIGSEGV/SIGABRT)
           the exit status will be non-zero or signal-terminated */
        char cmd[512];
        snprintf(cmd, sizeof(cmd),
                 "timeout 5 makecvmc/src/makecvmc %s %s 2>/dev/null; echo $?",
                 tmpfile, outfile);

        FILE *proc = popen(cmd, "r");
        ck_assert_ptr_nonnull(proc);
        char result[16] = {0};
        fread(result, 1, sizeof(result)-1, proc);
        int ret = pclose(proc);

        /* The process must not have been killed by a signal (segfault = 139) */
        ck_assert_msg(WIFEXITED(ret),
                      "payload %d: process killed by signal (likely buffer overflow)", i);

        unlink(tmpfile);
        unlink(outfile);
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("BufferOverflow");
    tcase_set_timeout(tc_core, 30);
    tcase_add_test(tc_core, test_ihx_buffer_overflow);
    suite_add_tcase(s, tc_core);
    return s;
}

int main(void)
{
    int number_failed;
    Suite *s = security_suite();
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}