#include "fs.h"
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// las mascaras en 0 = vacias
void
inicializar_vfsf(vsfs_t *vsfs)
{
	for (int i = 0; i < MAX_INODOS; i++) {
		vsfs->mascara_inodos[i] = 0;
	}
}

void inicializar_directorio(directorio_t *directorio) {
    directorio->cantidad_entradas=0;
    for(int i=0; i<MAX_DIR_ENTRADAS;i++) {
        directorio->entradas[i].ino = -1;
    }
}

void
crear_raiz(vsfs_t *vsfs)
{
	// llenar el inodo
	vsfs->inodos[0].gid = getgid();
	vsfs->inodos[0].uid = getuid();
	vsfs->inodos[0].ino = INODO_RAIZ;
	vsfs->inodos[0].modo = __S_IFDIR | 0664;
	vsfs->inodos[0].blocks = 8;
	vsfs->inodos[0].size = TAM_BLOQUE;
	vsfs->inodos[0].links = 2;
	vsfs->inodos[0].datos = 0;
	vsfs->inodos[0].padre = 0; // es padre de si mismo!!!
	// marcar ese inodo como ocupado
	vsfs->mascara_inodos[0] = 1;
	// fechas
	vsfs->inodos[0].atime =  ahora();
	vsfs->inodos[0].mtime =  ahora();
	vsfs->inodos[0].ctime =  ahora();

    //inicializar en -1 los dentries
    inicializar_directorio((directorio_t*)&vsfs->bloques_datos[0]);
    
	// ponerle el nombre "/"
	directorio_t *raiz = &((directorio_t *)&vsfs->bloques_datos[0])[0];
    raiz->entradas[0].ino = INODO_RAIZ;
    strcpy(raiz->entradas[0].nombre, "/");
	raiz->cantidad_entradas = 1;
}

int inodo_libre(vsfs_t *vsfs) {
	
	char *inodos = vsfs->mascara_inodos;
	for (int i = 0; i < MAX_INODOS; i++) {
		if (inodos[i] == 0) {
			return i;
		}
	}
	
	return -1;
}

int pos_libre_en_dir(directorio_t *dir) {
	for (int i = 0; i < MAX_DIR_ENTRADAS; i++) {
		if (dir->entradas[i].ino < 0) {
			return i;
		}
	}
	
	return -1;
}

void crear_directorio(int ino_padre, int inodo, const char *nombre, vsfs_t *fs, mode_t modo)
{
	if (inodo < 0 || inodo > MAX_INODOS) {
		return;
	}
	// marcar ese inodo como ocupado
	fs->mascara_inodos[inodo] = 1;
	fs->inodos[inodo].gid = getgid();
	fs->inodos[inodo].uid = getuid();
	fs->inodos[inodo].ino = inodo;
	fs->inodos[inodo].blocks = 8;
	fs->inodos[inodo].modo = modo | 0664;
	fs->inodos[inodo].size = TAM_BLOQUE;
	fs->inodos[inodo].links = 2;
	fs->inodos[inodo].datos = inodo;
	fs->inodos[inodo].padre = ino_padre;
	// fechas
	fs->inodos[inodo].atime =  ahora();
	fs->inodos[inodo].mtime =  ahora();
	fs->inodos[inodo].ctime =  ahora();
    inicializar_directorio((directorio_t*)&fs->bloques_datos[inodo]);

    // crear la entrada en el directorio padre
    // ponerle el nombre 
    directorio_t *padre = &((directorio_t *)&fs->bloques_datos[ino_padre])[0];
    int pos_libre = pos_libre_en_dir(padre);
    padre->entradas[pos_libre].ino = inodo;
    strcpy(padre->entradas[pos_libre].nombre, nombre);
	// aumentar la cantidad de archivos
	padre->cantidad_entradas++;
	// actualizar metadata en el padre
	inodo_actualizar_fecha_cambio(&fs->inodos[ino_padre],ahora());
	inodo_actualizar_fecha_modificacion(&fs->inodos[ino_padre],ahora());
	inodo_actualizar_fecha_acceso(&fs->inodos[ino_padre],ahora());
}

void crear_archivo(int ino_padre, int inodo, const char *nombre, 
const char *contenido, vsfs_t *fs, mode_t modo) {
	
	if (inodo < 0 || inodo > MAX_INODOS) {
		return;
	}
	
	// marcar ese inodo como ocupado
	fs->mascara_inodos[inodo] = 1;
	fs->inodos[inodo].gid = getgid();
	fs->inodos[inodo].uid = getuid();
	fs->inodos[inodo].ino = inodo;
	fs->inodos[inodo].blocks = 8;
	fs->inodos[inodo].modo = modo | 0664;
	fs->inodos[inodo].size = 0;
	fs->inodos[inodo].links = 1;
	fs->inodos[inodo].datos = inodo;
	fs->inodos[inodo].padre = ino_padre;
	// fechas
	fs->inodos[inodo].atime =  ahora();
	fs->inodos[inodo].mtime =  ahora();
	fs->inodos[inodo].ctime =  ahora();
	// fs->inodos[inodo].btime =  ahora();

    // crear la entrada en el directorio padre
    // ponerle el nombre 
    directorio_t *padre = &((directorio_t *)&fs->bloques_datos[ino_padre])[0];
    int pos_libre = pos_libre_en_dir(padre);
    padre->entradas[pos_libre].ino = inodo;
    strcpy(padre->entradas[pos_libre].nombre, nombre);
	// aumentar la cantidad de archivos
	padre->cantidad_entradas++;
	// actualizar metadata en el padre
	inodo_actualizar_fecha_cambio(&fs->inodos[ino_padre],ahora());
	inodo_actualizar_fecha_modificacion(&fs->inodos[ino_padre],ahora());
	inodo_actualizar_fecha_acceso(&fs->inodos[ino_padre],ahora());
}

void eliminar_inodo(vsfs_t *fs ,int indice_inodo) {

	// marcar inodo como libre
    fs->mascara_inodos[indice_inodo] = INODO_LIBRE;
	// eliminar entrada en el padre
	int inodo_padre = fs->inodos[indice_inodo].padre;
	directorio_t *padre = &((directorio_t *)&fs->bloques_datos[inodo_padre])[0];
	padre->cantidad_entradas--;
	for (int i=0; i<MAX_DIR_ENTRADAS; i++) {
		if(padre->entradas[i].ino == indice_inodo) {
			padre->entradas[i].ino = -1;
			for(int j=0; j< strlen(padre->entradas[i].nombre); j++) {
				padre->entradas[i].nombre[j] = 0;
			}
			// actualizar metadata en el padre
			inodo_actualizar_fecha_cambio(&fs->inodos[inodo_padre],ahora());
			inodo_actualizar_fecha_modificacion(&fs->inodos[inodo_padre],ahora());
			inodo_actualizar_fecha_acceso(&fs->inodos[inodo_padre],ahora());
			break;
		}
	}
	
}

// Utilidades:

int encontrar_padre(int inodo, vsfs_t *fs) {
    
	if(inodo < 0) {
        return -1;
    }
    
	return fs->inodos[inodo].padre;
}

char* segmento_de_path(const char *path) {
    
	printf("[INFO] path = %s\n", path);
	if (path == NULL) {
		return NULL;
	}

	int longitud = 0;
	for (int i = 0; i < strlen(path); i++) {
		if (path[i] == '/') {
			break;
		}
		longitud++;
	}
	
	char *segmento = malloc(sizeof(char) * (longitud + 1));
	segmento[longitud] = 0;
	strncpy(segmento, path, longitud);
    printf("[INFO] segmento antes de salir = %s\n", segmento);
	
	return segmento;
}

int ultimo_segmento_de_path(const char * path) {
    if(strcmp(path, "/") ==0) {
        return 0;
    }
    int l = strlen(path);
    int f = 0;
    for(int i = l-1; i>=0; i--) {
        if(path[i]=='/') {
            break;
        }
        f++;
    }    
    return l -f;
}

int dame_inodo_de_este_path(const char *path, vsfs_t *fs, int inodo, int *ultimo_padre) {
    printf("[INFO] dentro de dame_el_inodo de %s\n", path);
    //inicio recorriendo la raiz
    if(strcmp(path, "/") == 0) {
        return 0;
    } else {
        // buscar en la raiz el path
        char * nuevo_path = segmento_de_path(&path[1]);

        for(int i=0; i<MAX_DIR_ENTRADAS; i++) {
            if((((directorio_t*)fs->bloques_datos[inodo])[0].entradas[i].ino) >-1) {
                if(strcmp(nuevo_path, ((directorio_t*)fs->bloques_datos[inodo])[0].entradas[i].nombre)==0) {
                    // ver si es la ultima parte del path
                    *ultimo_padre = inodo;
                    if(strcmp(&path[strlen(nuevo_path)+1], "")==0) {
                        return ((directorio_t*)fs->bloques_datos[inodo])[0].entradas[i].ino;

                    } else { // seguir buscando
                        *ultimo_padre = inodo;
                        return dame_inodo_de_este_path( &path[strlen(nuevo_path)+1], fs, 
						((directorio_t*)fs->bloques_datos[inodo])[0].entradas[i].ino,  ultimo_padre);
                    }
                }
            }
        }
        printf("[INFO] path '%s' no existe\n", nuevo_path);
        *ultimo_padre = inodo;
        // nuevo_path no esta en la raiz
        
		return -1;
    }
}

unsigned long ahora() {
	return (unsigned long)time(NULL);
}


void inodo_actualizar_fecha_cambio(inodo_t *inodo, unsigned long tv) {
	if(inodo == NULL) {
		return;
	}
	
	inodo->ctime = tv;
}

void inodo_actualizar_fecha_modificacion(inodo_t *inodo, unsigned long tv) {
	if(inodo == NULL) {
		return;
	}
	
	inodo->mtime = tv;
}

void inodo_actualizar_fecha_acceso(inodo_t *inodo, unsigned long tv) {
	if(inodo == NULL) {
		return;
	}
	
	inodo->atime = tv;
}