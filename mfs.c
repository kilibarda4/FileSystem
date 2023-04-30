#define _GNU_SOURCE 1

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 1024
#define BLOCKS_PER_FILE 1024

#define MAX_FILE_LEN 64
#define NUM_FILES 256

#define DISK_IMAGE_SIZE 67108864
#define NUM_BLOCKS (DISK_IMAGE_SIZE) / (BLOCK_SIZE)

// No command has more than 5 arguments in our
// list of commands
#define MAX_NUM_ARGUMENTS 5
#define MAX_COMMAND_SIZE 255

uint8_t curr_image[NUM_BLOCKS][BLOCK_SIZE];

struct directoryEntry
{
    char filename[64];
    bool in_use;
    int32_t inode;
};

struct directoryEntry *directory;
struct inode *inodes;

struct inode
{
    int32_t blocks[BLOCKS_PER_FILE];
    bool in_use;
};

// Command stuff
typedef void (*command_fn)(char *[MAX_NUM_ARGUMENTS]);

typedef struct _command
{
    char *name;
    command_fn run;
    uint8_t num_args;
} command;

// Add the commands here!
void insert(char *tokens[MAX_NUM_ARGUMENTS])
{
}
void retrieve(char *tokens[MAX_NUM_ARGUMENTS])
{
}
void read(char *tokens[MAX_NUM_ARGUMENTS])
{
}
void del(char *tokens[MAX_NUM_ARGUMENTS])
{
}
void undel(char *tokens[MAX_NUM_ARGUMENTS])
{
}
void list(char *tokens[MAX_NUM_ARGUMENTS])
{
}
void df(char *tokens[MAX_NUM_ARGUMENTS])
{
}
void openfs(char *tokens[MAX_NUM_ARGUMENTS])
{
}
void closefs(char *tokens[MAX_NUM_ARGUMENTS])
{
}
void createfs(char *tokens[MAX_NUM_ARGUMENTS])
{
}
void savefs(char *tokens[MAX_NUM_ARGUMENTS])
{
}
void attrib(char *tokens[MAX_NUM_ARGUMENTS])
{
}
void encrypt(char *tokens[MAX_NUM_ARGUMENTS])
{
}
void decrypt(char *tokens[MAX_NUM_ARGUMENTS])
{
}

#define NUM_COMMANDS 14
const command commands[NUM_COMMANDS] = {
    {"insert", insert, 1},
    {"retrieve", retrieve, 2},
    {"read", read, 3},
    {"delete", del, 1},
    {"undel", undel, 1},
    {"list", list, 0},
    {"df", df, 0},
    {"open", openfs, 1},
    {"close", closefs, 0},
    {"createfs", createfs, 1},
    {"savefs", savefs, 0},
    {"attrib", attrib, 1},
    {"encrypt", encrypt, 2},
    {"decrypt", decrypt, 2}
};

///////////////////////////////////////
// Forward declarations
//////////////////////////////////////
void parse_tokens(const char *command_string, char **token);
void free_array(char **arr, size_t size);

void init()
{
    directory = (struct directoryEntry *)&curr_image[0][0];
    inodes = (struct inode *)&curr_image[20][0];

    for (int i = 0; i < NUM_FILES; ++i)
    {
        directory[i].in_use = 0;
        directory[i].inode = -1;
        for (int j = 0; j < BLOCKS_PER_FILE; ++j)
        {
            inodes[i].blocks[j] = -1;
        }
        inodes[i].in_use = 0;
    }
}

int main(int argc, char **argv)
{
    char *command_string = (char *)malloc(MAX_COMMAND_SIZE);
    char *tokens[MAX_NUM_ARGUMENTS] = {NULL};

    int i;

    while (1)
    {
        // Print out the msh prompt
        printf("mfs> ");

        // Read the command from the commandline.  The
        // maximum command that will be read is MAX_COMMAND_SIZE
        // This while command will wait here until the user
        // inputs something since fgets returns NULL when there
        // is no input
        while (!fgets(command_string, MAX_COMMAND_SIZE, stdin))
            ;

        // Ignore blank lines
        if (*command_string == '\n')
        {
            continue;
        }

        parse_tokens(command_string, tokens);

        char *cmd = tokens[0];

        // Quit if command is 'quit' or 'exit'
        if (!strcmp(cmd, "quit") || !strcmp(cmd, "exit"))
        {
            break;
        }

        for (i = 0; i < NUM_COMMANDS; ++i)
        {
            if (! strcmp(cmd, commands[i].name))
            {
                if (tokens[commands[i].num_args] == NULL)
                {
                    fprintf(stderr, "%s: Not enough arguments\n", cmd);
                    break;
                }
                commands[i].run(tokens);
                break;
            }
        }

        if (i == NUM_COMMANDS)
        {
            fprintf(stderr, "%s: Invalid command `%s'\n", argv[0], cmd);
        }
    }

    free(command_string);
    free_array(tokens, MAX_NUM_ARGUMENTS);

    return 0;
}

void free_array(char **arr, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        if (arr[i] != NULL)
        {
            free(arr[i]);
        }
    }
}

void parse_tokens(const char *command_string, char **token)
{
#define WHITESPACE " \t\n"

    // Clean up the old values in token
    free_array(token, MAX_NUM_ARGUMENTS);

    int token_count = 0;
    // Pointer to point to the token parsed by strsep
    char *argument_ptr = NULL;

    char *working_string = strdup(command_string);

    // we are going to move the working_string pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *head_ptr = working_string;

    // Tokenize the input strings with whitespace used as the delimiter
    while (((argument_ptr = strsep(&working_string, WHITESPACE)) != NULL) &&
           (token_count < MAX_NUM_ARGUMENTS))
    {
        token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);
        if (strlen(token[token_count]) == 0)
        {
            free(token[token_count]);
            token[token_count] = NULL;
        }
        token_count++;
    }

    // Set all args from the last arg parsed to the end of the token array
    // This helps so that I don't get a double free error if I run a cmd that
    // has 7 args then run one that has 3 args then called free_array on token
    for (; token_count < MAX_NUM_ARGUMENTS; ++token_count)
    {
        token[token_count] = NULL;
    }

    free(head_ptr);
}