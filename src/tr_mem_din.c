/**
 * Universidad de Murcia
 *
 * Ampliación de Sistemas Operativos
 * Autor: Victorio Jesús Molina Bermejo
 * DNI: 48632380-F
 */

#define _POSIX_C_SOURCE 200809L
#define DEFAULT_BUF_SIZE 1024
#define MIN_BUF_SIZE 1
#define MAX_BUF_SIZE 134217728

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

/* 
    Notas: Por defecto, se escribe en 'stdout' y se lee de 'stdin'.
*/

void replace_str_chars(char *buf, char *chars_src, char *chars_dst, int n)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < strlen(chars_src); j++)
        {
            // Reemplazamos el caracter de src por dst
            if (buf[i] == chars_src[j])
            {
                buf[i] = chars_dst[j];
            }
        }
    }
}

int write_all(int fdout, char *buf, unsigned buf_size)
{
    int size = buf_size;
    ssize_t num_written;
    char *p = buf;
    while (size > 0 && (num_written = write(fdout, p, size)) != -1)
    {
        p += num_written;
        size -= num_written;
    }

    if (size > 0)
    {
        perror("write()");
        exit(EXIT_FAILURE);
    }

    return buf_size;
}

void trfd(int fdin, int fdout, char *buf, char *chars_src, char *chars_dst, unsigned buf_size)
{
    ssize_t num_read, num_written;

    while ((num_read = read(fdin, buf, buf_size)) > 0)
    {
        replace_str_chars(buf, chars_src, chars_dst, num_read);
        num_written = write_all(fdout, buf, num_read);
        if (num_written == -1)
        {
            perror("write(fdin)");
            exit(EXIT_FAILURE);
        }
        /* Escrituras parciales no tratadas */
        assert(num_written == num_read);
    }

    if (num_read == -1)
    {
        perror("read(fdin)");
        exit(EXIT_FAILURE);
    }

    if (close(fdin) == -1)
    {
        perror("close(fdin)");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    int opt;
    char *chars_src = NULL;
    char *chars_dst = NULL;
    size_t buf_size = DEFAULT_BUF_SIZE;
    char *fileout = NULL;
    int fdout;
    int fdin;
    char *buf;

    optind = 1;
    while ((opt = getopt(argc, argv, "s:d:t:o:h")) != -1)
    {
        switch (opt)
        {
        case 's':
            chars_src = optarg;
            break;
        case 'd':
            chars_dst = optarg;
            break;
        case 't':
            buf_size = atoi(optarg);
            break;
        case 'o':
            fileout = optarg;
            break;
        case 'h':
        default:
            fprintf(stderr, "USO: %s -s SRC -d DST [-t SIZE] [-o FILEOUT] [FILEIN1 FILEIN2 ... FILEINn]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    /* Las cadenas deben ser no nulas y tener la misma longitud, mayor o igual que 1 */
    if (!chars_src || !chars_dst)
    {
        fprintf(stderr, "USO: %s -s SRC -d DST [-t SIZE] [-o FILEOUT] [FILEIN1 FILEIN2 ... FILEINn]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    else
    {
        int chars_src_len = strlen(chars_src);
        int chars_dst_len = strlen(chars_dst);

        if (!chars_src_len || !chars_dst_len)
        {
            fprintf(stderr, "Error: SRC y DST deben ser cadenas de caracteres de longitud mayor o igual a 1\n");
            exit(EXIT_FAILURE);
        }
        else if (chars_src_len != chars_dst_len)
        {
            fprintf(stderr, "Error: SRC y DST deben ser cadenas de caracteres de la misma longitud\n");
            exit(EXIT_FAILURE);
        }
    }

    /* El tamaño del buffer no puede ser menor que MIN_BUF_SIZE, ni mayor que MAX_BUF_SIZE */
    if (buf_size < MIN_BUF_SIZE || buf_size > MAX_BUF_SIZE)
    {
        fprintf(stderr, "Error: SIZE debe ser un valor entre %d y %d\n", MIN_BUF_SIZE, MAX_BUF_SIZE);
        exit(EXIT_FAILURE);
    }

    /* Abre el fichero de salida */
    if (fileout != NULL)
    {
        fdout = open(fileout, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fdout == -1)
        {
            perror("open(fileout)");
            exit(EXIT_FAILURE);
        }
    }
    else /* Por defecto, la salida estándar */
        fdout = STDOUT_FILENO;

    /* Reserva memoria dinámica para buffer de lectura */
    if ((buf = (char *)malloc(buf_size * sizeof(char))) == NULL)
    {
        perror("malloc()");
        exit(EXIT_FAILURE);
    }

    /* Abre cada fichero de entrada y lo escribe en 'fileout' */
    if (optind < argc)
        for (int i = optind; i < argc; i++)
        {
            int fdin = open(argv[i], O_RDONLY);
            if (fdin == -1)
            {
                perror("open(filein)");
                continue;
            }
            trfd(fdin, fdout, buf, chars_src, chars_dst, buf_size);
        }
    else
    {
        trfd(STDIN_FILENO, fdout, buf, chars_src, chars_dst, buf_size);
    }

    if (close(fdout) == -1)
    {
        perror("close(fdout)");
        exit(EXIT_FAILURE);
    }

    /* Libera memoria dinámica de buffer de lectura */
    free(buf);

    exit(EXIT_SUCCESS);
}
