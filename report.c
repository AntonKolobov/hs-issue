#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <hs.h>

#define FLAGS    HS_FLAG_DOTALL

int main(int argc, char** argv)
{
    const char* set[] = {
	"ftyp",
	"^.{4}1\\.0 Fri Feb 3 09:55:56 MET 1995",
	"^# Bazaar merge directive format",
	"^SC68 Music-file \\/ \\(c\\) \\(BeN\\)jami",
	"^\\*\\* This file contains an SQLite",
	"^Cobalt Networks Inc\\.\\nFirmware v",
	"^\\<Maker\\x20Intermediate\\x20Print\\x20File",
	"^.{2}---BEGIN\\x20PGP\\x20PUBLIC\\x20KEY\\x20BLOCK-",
	"^SNES-SPC700 Sound File Data v",
	"^.{11}must be converted with BinHex",
	"^.{64}\\x00{16}"    // This regexp slows matching extremely
    };

    const int mode_slow = (argc == 1);
    const size_t SET_SZ = mode_slow ? sizeof(set)/sizeof(const char*) : sizeof(set)/sizeof(const char*) - 1;

    struct hs_database* db;
    struct hs_scratch* scratch = NULL;
    int ids[SET_SZ];
    int flags[SET_SZ];

    int i;
    for (i = 0; i < SET_SZ; ++i) {
	ids[i] = i;
	flags[i] = FLAGS;
    }

    /// DBs COMPILATION
    hs_compile_error_t* compileErr;
    clock_t begin = clock();
    hs_error_t err = hs_compile_multi(set, flags, ids,
                           SET_SZ, HS_MODE_BLOCK,
                           NULL, &db, &compileErr);
    if (err != HS_SUCCESS) {
	printf("Failed to compile the DB:\n%s\n", compileErr->message);
        return -1;
    }
    err = hs_alloc_scratch(db, &scratch);
    if (err != HS_SUCCESS) {
	printf("Failed to allocate scratch:\n%s\n", compileErr->message);
        return -1;
    }
    hs_free_compile_error(compileErr);
    clock_t end = clock();
    double spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("[DB compilation]: %.2f seconds spent\n", spent);

    /// READ THE TEST DATA
    const size_t buf_len = 512;
    char buffer[buf_len];
    int fd = open("./test_data", O_RDONLY);
    if (fd == -1) {
	perror ("Failed to open data file: ");
        return -2;
    }
    size_t rd = read(fd, buffer, buf_len);
    if (rd != buf_len) {
	printf("Failed to read file data.\n");
	return -2;
    }
    close(fd);

    /// DO THE SCANNING
    size_t niters = 1e6;
    begin = clock();
    while (niters--)
	hs_scan(db, buffer, buf_len, 0, scratch, NULL, NULL);
    end = clock();
    spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("[Matching]: %.2f seconds spent\n", spent);
    
    return 0;
}
  
