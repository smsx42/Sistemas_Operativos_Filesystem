#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


#define MAX_INODOS 110
#define MAX_DIRS 10
#define MAX_NOMBRE_ARCHIVO 255
#define MAX_DIR_ENTRADAS 110
#define TAM_BLOQUE 4096
#define INODO_RAIZ 0
#define INODO_LIBRE 0
#define INODO_OCUPADO 1



// Una entrada tiene un numero de inodo y el nombre del archivo.
typedef struct dirent {
    int ino;
    char nombre[MAX_NOMBRE_ARCHIVO];
} dirent_t;

// Cada directorio puede tener X entradas (archivos)
typedef struct directorio {
    int cantidad_entradas;
    dirent_t entradas[MAX_DIR_ENTRADAS];// rever esta cantidad.  
} directorio_t;

// Cada archivo puede ocupar 1 solo bloque de 4k = 4096.
typedef struct inodo {
    int ino; // numero de inodo.
    int size; // en bytes.
    int blocks; // candidad de bloques que ocupan los datos.
    int modo; // dir o archivo regular.
    int links;
    int uid;
    int gid;
    int datos; // el numero de bloque donde estan los datos.
    int padre; // el inodop.
    char *path_inodo;
    unsigned long atime; // fecha de acceso.
    unsigned long mtime; // fecha de cambio en el contenido.
    unsigned long ctime; // fecha de cambio en la metadata del archivo.
} inodo_t;


// Todo el Filesystem.
typedef struct vsfs {

    char mascara_inodos[MAX_INODOS]; // Me dice cuando un inodo esta ocupado.
    inodo_t inodos[MAX_INODOS]; // Los inodos pueden ser archivos o dirs.
    void * bloques_datos[MAX_INODOS][TAM_BLOQUE];
} vsfs_t;

// Inicializa el Filesystem.
void inicializar_vfsf(vsfs_t *vsfs);

// Inicializa un direcotiro.
void inicializar_directorio(directorio_t *directorio);

// Crea la raiz direcotrio del Filesystem.
void crear_raiz(vsfs_t *vsfs);

// Elimina un inodo dado su indice.
void eliminar_inodo(vsfs_t *fs ,int indice_inodo);

// Crea un inodo del tipo direcotorio.
void crear_directorio(int ino_padre, int inodo, const char *nombre, vsfs_t *fs, mode_t modo);

// Crea un inodo del tipo regular.
void crear_archivo(int ino_padre, int inodo, const char *nombre, const char *contenido, vsfs_t *fs, mode_t modo);

// Utilidades:

// Devuelve el primer inodo libre segun el bitmap de inodos del Filesystem y 
// devuelve -1 en caso de que no se retorne un inodo.
int inodo_libre(vsfs_t *vsfs);

// Devuelve la primera entrada libre dentro de un directorio del FIlesystem y 
// devuelve -1 en caso de que no se retorne una entrada.
int pos_libre_en_dir(directorio_t *dir);

// Devuelve el segmento de un path dado.
char * segmento_de_path(const char *path);

// Devuelve el ultimo segmento de un path dado.
int ultimo_segmento_de_path(const char * path);

int encontrar_padre(int inodo, vsfs_t *fs); // Creo que no se usa.

// Devuelve el inodo que se encuntra ubicado en un determinado path.
int dame_inodo_de_este_path(const char *path, vsfs_t *fs, int inodo, int *ultimo_padre);

// Devuelve el tiempo actual.
unsigned long ahora();

void actualiza_acceso(vsfs_t *fs, int inodo, unsigned long tv); // Creo que no se usa

// Actualiza fecha de cambio del inodo.
void inodo_actualizar_fecha_cambio(inodo_t *inodo, unsigned long tv);

// Actualiza fecha de modificacion del inodo.
void inodo_actualizar_fecha_modificacion(inodo_t *inodo, unsigned long tv);

// Actualiza fecha de accesodel inodo.
void inodo_actualizar_fecha_acceso(inodo_t *inodo, unsigned long tv);