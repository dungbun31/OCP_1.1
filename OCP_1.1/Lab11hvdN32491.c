#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <getopt.h>

bool has_substring(unsigned char *str1, unsigned char *str2);
void search_files_recursive(const char *dir_path, unsigned char *hexFind, char *DEBUG);
void search_hex_in_file(char *filePath, unsigned char *hexFind, char *DEBUG);
void handleOptions(int argc, char *argv[]);

int count = 0;

int main(int argc, char *argv[])
{
    handleOptions(argc, argv);
    if ((argc == 1) || (argc > 3))
    {
        printf("Wrong number of arguments! \n\n");
        printf("Use: -h (--help) to see how the program works\n\n");
        return EXIT_FAILURE;
    }
    int len = strlen(argv[2]);
    if (len % 2 == 1)
    {
        printf("Wrong byte sequence input");
        return EXIT_FAILURE;
    }

    char *DEBUG = getenv("LAB11DEBUG");
    if (DEBUG)
    {
        printf("*\n");
        printf("The program searches for the sequence of bytes %s in files, starting from the directory %s \n", argv[2], argv[1]);
        printf("*\n");
    }
    unsigned char *hexFind = (unsigned char *)argv[2];

    search_files_recursive(argv[1], hexFind, DEBUG);

    if (DEBUG)
    {
        printf("*\n");
        printf("Number of files: %d.\n", count);
        printf("*\n");
        printf("The program has ended. \n");
    }
    return EXIT_SUCCESS;
}

void search_files_recursive(const char *dir_path, unsigned char *hexFind, char *DEBUG)
{
    DIR *dir = opendir(dir_path);

    if (dir == NULL)
    {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char filePath[PATH_MAX];
        snprintf(filePath, sizeof(filePath), "%s/%s", dir_path, entry->d_name);

        struct stat file_stat;
        if (stat(filePath, &file_stat) == -1)
        {
            perror("stat");
            continue;
        }

        if (S_ISREG(file_stat.st_mode))
        {
            search_hex_in_file(filePath, hexFind, DEBUG);
        }
        else if (S_ISDIR(file_stat.st_mode))
        {
            search_files_recursive(filePath, hexFind, DEBUG);
        }
    }

    closedir(dir);
}

void search_hex_in_file(char *filePath, unsigned char *hexFind, char *DEBUG)
{
    FILE *file = fopen(filePath, "rb");
    if (file == NULL)
    {
        return;
    }

    fseek(file, 0, SEEK_END);
    unsigned long bufferSize = ftell(file);
    rewind(file);

    unsigned char *buffer = (unsigned char *)malloc(bufferSize);
    if (buffer == NULL)
    {
        fclose(file);
        return;
    }

    size_t result = fread(buffer, 1, bufferSize, file);
    if (result != bufferSize)
    {
        free(buffer);
        fclose(file);
        return;
    }

    unsigned char *hex_data = (unsigned char *)malloc(2 * bufferSize + 1);
    if (hex_data == NULL)
    {
        free(buffer);
        fclose(file);
        return;
    }

    unsigned char *p = hex_data;

    for (unsigned long i = 0; i < bufferSize; i++)
    {
        p += sprintf((char *)p, "%02x", buffer[i]);
    }

    *p = '\0';

    unsigned char *hex_str = hexFind + 2;

    char *position = strstr((const char *)hex_data, (const char *)hex_str);

    if (position != NULL)
    {
        if (DEBUG)
        {
            size_t offset = ((unsigned char *)position - hex_data) / 2;
            printf("%s is in file %s at offset %zu from the beginning of the file \n\n", hexFind, filePath, offset);
            count++;
        }
        else
        {
            printf("%s\n", filePath);
            count++;
        }
    }

    free(hex_data);
    free(buffer);
    fclose(file);
}

void handleOptions(int argc, char *argv[])
{
    const char *short_options = "h::v::";
    const struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        {NULL, 0, NULL, 0}};

    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1)
    {
        switch (c)
        {
        case 'h':
            printf("lab11hvdN32491: ./lab11hvdN32491 [OPTION]\n");
            printf("                ./lab11hvdN32491 [directory] [target]\n");
            printf("Arguments:\n");
            printf("- directory: The path from which the search begins\n");
            printf("- target: byte sequence in file need to find \n\n");
            printf("Options:\n");
            printf("  -h, --help     Print help message\n");
            printf("  -v, --version  Print version information \n");
            exit(EXIT_SUCCESS);
            break;

        case 'v':
            printf("Лабораторная работа 1.1\n");
            printf("Хоанг Вьет Зунг\n");
            printf("Вариант 2, нечетный\n");

            exit(EXIT_SUCCESS);
            break;

        case '?':
            exit(EXIT_FAILURE);
            break;

        default:
            printf("Use -h (--help) to see how the program works.\n");
            exit(EXIT_FAILURE);
            break;
        }
    }
}
