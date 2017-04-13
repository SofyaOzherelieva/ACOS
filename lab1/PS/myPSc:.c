#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>


typedef struct {
    int pid;
    char *CMD;
    char *tty;
    char *qwe;
    int stime, utime;
    int totalTime;
} procps_status_t;

// Сортирует файлы в директории в алфавитном порядке.
void sortDir( DIR* proc )
{
    struct dirent **forSort;
    scandir( "/proc", &forSort, 0, alphasort );
}

// Возвращает слово после пробела n.
char* pars( int n, char* state )
{
    int i = 0;
    int numberOfSpaces = 0;
    int length = 0;
    while( state[i] != '\0' ) {
        if( state[i] == ' ' ) {
            numberOfSpaces += 1;
            i += 1;
        }
        if( numberOfSpaces == n ) {
            while( state[i] != ' ' && state[i] != '\0' ) {
                i += 1;
                length += 1;
            }
            i = i - length;
            char * string = (char*) malloc( ( length + 1 ) * sizeof( char ) );
            int u = 0;
            for( u = 0; u <= length; u++ ) {
                string[u] = '\0';
            }
            int j = 0;
            if( state[i] == '(' ) {
                i += 1;
                length -= 1;
            }
            if( state[i + length - 1] == ')' ) {
                length -= 1;
            }
            for( j = 0; j < length; j++ ) {
                // Избавляемся от скобок.
                string[j] = state[i];
                i += 1;
            }
            i -= 1;
            return string;
        }
        i += 1;
    }
}

char* showStat( char* pid )
{
    FILE* process;

    // Формируем адрес proc/<pid>/stat
    char* nameStat = (char*) malloc( ( 6 + 5 + 5 + 1 ) * sizeof( char ) );
    nameStat[0] = '\0';
    strncat( nameStat, "/proc/", 6 );
    strncat( nameStat, pid, 5 );
    strncat( nameStat, "/stat", 5 );
    process = fopen( nameStat, "r" );

    // Считаем кол-во символов в строке stat.
    int current = 0;
    int number = 0;
    while( ( current = fgetc( process ) ) != EOF ) {
        number += 1;
    }
    rewind( process );
    char* state = (char*) malloc( ( number + 1 ) * sizeof( char ) );

    fgets( state, number + 1, process );
    free( nameStat );
    fclose( process );
    return state;
}


char* find_tty( char* tty_number )
{
    int find = atoi( tty_number );
    int length = 4 + 1 + 1;
    char* tty = (char*) malloc( ( length + 1 ) * sizeof( char ) );
    int i = 0;
    for( i = 0; i < length + 1; i++ ) {
        tty[i] = '\0';
    }

    if( ( ( find >> 8 ) & 136 ) != 0 ) {
        strncat( tty, "pts/", 4 );
    } else {
        if( ( ( find >> 8 ) & 4 ) != 0 ) {
            strncat( tty, "tty/", 4 );
        } else {
            strncat( tty, "?", 1 );
        }
    }
    if( tty[0] != '?' ) {
        char* tty_nr = (char*) malloc( 2 * sizeof( char ) );
        int i = 0;
        for( i = 0; i < 2; i++ ) {
            tty_nr[i] = '\0';
        }
        int find1 = ( find & 255 );
        tty_nr[0] = find1 + '0';
        strncat( tty, tty_nr, 1 );
        free( tty_nr );
    }
    return tty;
}

// Обрабатывает каждый файл c процессом.
void readEachFile( DIR* proc )
{
    struct dirent *filesList;
    procps_status_t currentStatus;
    char* pid;

    while( ( filesList = readdir( proc ) ) != NULL ) {
        pid = filesList->d_name;

        // Если название это не PID процесса, пропускаем.
        if( !( pid[0] >= '0' && pid[0] <= '9' ) ) {
            continue;
        }

        // Записываем PID процесса
        currentStatus.pid = atoi( pid );

        char* state = showStat( pid );

        currentStatus.CMD = pars( 1, state );

        //Вычисляем время работы. 
        currentStatus.utime = atoi( pars( 13, state ) );
        currentStatus.stime = atoi( pars( 14, state ) );

        currentStatus.tty = find_tty( pars( 6, state ) );

        currentStatus.totalTime = ( currentStatus.utime + currentStatus.stime ) / sysconf( _SC_CLK_TCK );

        int ss = currentStatus.totalTime % 60;
        int mm = ( currentStatus.totalTime % 3600 ) / 60;
        int hh = currentStatus.totalTime / 3600;

        printf( "%5d %-8s %02u:%02u:%02u %-s \n", currentStatus.pid, currentStatus.tty,
            hh, mm, ss, currentStatus.CMD );
        free( currentStatus.CMD );
        free( currentStatus.tty );
        free( state );
    }
}

int main()
{
    printf( " %4s %2s %13s %2s \n", "PID", "TTY", "TIME", "CMD" );
    DIR *proc = opendir( "/proc" );
    sortDir( proc );

    readEachFile( proc );

    closedir( proc );
}