#include <asm-generic/errno-base.h>
#include <time.h>
#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "fs.h"

#define DEFAULT_FILE_DISK "persistence_file.fisopfs"

char *filedisk = DEFAULT_FILE_DISK;

// crear el filesystem global
vsfs_t el_fs;

// Modifica la fecha de la ultima modificacion y de la ultima 
// fecha de acceso de un archivo, retornando:
// return -ENOENT en caso de que no exista el inodo.
// 0 en caso de exito.
int fisopfs_utimens(const char *path, const struct timespec ts[2]) {
	printf("[debug] fisopfs_utimens - path: %s - time in seconds %li \n",
	       path,
	       ts->tv_sec);
	int padre = 0;
	int inodo = dame_inodo_de_este_path(path, &el_fs, 0, &padre);
	if (inodo == -1) {
		return -ENOENT;
	}
	// acceso
	if (ts[0].tv_sec > 0) {
		el_fs.inodos[inodo].atime = ts[0].tv_sec;
	}
	// modificacion
	if (ts[1].tv_sec > 0) {
		el_fs.inodos[inodo].mtime = ts[1].tv_sec;
	}
	
	return 0;
}

// Obtiene los atributos de un inodo, en doonde retorna:
// -ENOENT; en caso de que no exista el inodo.
// 0 en caso de exito.
// EN la consola se utiliza mediante el comando stat.
static int fisopfs_getattr(const char *path, struct stat *st) {
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	int x = 0;
	int inodo = dame_inodo_de_este_path(path, &el_fs, 0, &x);
	if (inodo < 0) {
		return -ENOENT;
	}
	inodo_t el_inodo = el_fs.inodos[inodo];
	st->st_uid = el_inodo.uid;
	st->st_gid = el_inodo.gid;
	st->st_nlink = el_inodo.links;
	st->st_blocks = el_inodo.blocks;
	st->st_size = el_inodo.size;
	st->st_mode = el_inodo.modo;
	st->st_atime = el_inodo.atime;
	st->st_mtime = el_inodo.mtime;
	st->st_ctime = el_inodo.ctime;
	
	return 0;
}

// Muestra el contenido de un directorio dado un path, en donde se guarda el contenido (buffer)
// y hasta donde debe mostrar (offset). Retorna:
// -ENOENT en caso de que no exista el inodo del tipo directorio.
// 0 en caso de exito.
static int fisopfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, 
off_t offset, struct fuse_file_info *fi) {
	
	printf("[debug] fisopfs_readdir - path: %s\n", path);

	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	int x = 0;
	int inodo = dame_inodo_de_este_path(path, &el_fs, 0, &x);
	if (inodo < 0) {
		return -ENOENT;
	}
	directorio_t *dir = &((directorio_t *) &el_fs.bloques_datos[inodo])[0];
	int inicio = 0;
	if (strcmp(path, "/") == 0) {
		inicio = 1;
	}
	for (int i = inicio; i < MAX_DIR_ENTRADAS; i++) {
		if ((dir->entradas[i].ino) > -1) {
			int l = strlen(dir->entradas[i].nombre);
			char *nombre = malloc(sizeof(char) * l + 1);
			nombre[l] = 0;
			strcpy(nombre, dir->entradas[i].nombre);
			filler(buffer, nombre, NULL, 0);
			// actualizar timestamps
			struct timespec tv[2];
			tv[0].tv_sec = ahora();
			tv[1].tv_sec = 0;
			fisopfs_utimens(path, tv);
		}
	}
	
	return 0;
}

// Lee un archivo en donde se necesita el path, en donde se guarda el contenido leido (buffer)
// el tamaño a leer del archivo, y la posicion hasta la que se leera (offset), en donde retorna:
// -ENOENT en caso de que no exista el inodo.
// size del archivo en caso de exito.
static int fisopfs_read(const char *path, char *buffer, 
size_t size, off_t offset, struct fuse_file_info *fi) {
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n", path, offset, size);

	int x = 0;
	int inodo = dame_inodo_de_este_path(path, &el_fs, 0, &x);
	if (inodo < 0) {
		return -ENOENT;
	}

	if (offset + size > strlen(&((char *) el_fs.bloques_datos[inodo])[0])) {
		size = strlen(&((char *) el_fs.bloques_datos[inodo])[0]) - offset;
	}

	size = size > 0 ? size : 0;

	memcpy(buffer, &((char *) el_fs.bloques_datos[inodo])[0] + offset, size);
	// actualizar timestamps
	struct timespec tv[2];
	tv[0].tv_sec = ahora();
	tv[1].tv_sec = 0;
	fisopfs_utimens(path, tv);
	
	return size;
}

// Crea un directorio con un nombre determinado, retornando 0 en caso de exito
// y -1 en caso contrario. Se utiliza el comando mkdir en la consola. 
int fisopfs_mkdir(const char *name, mode_t mode) {
	printf("[debug] fisopfs_mkdir - name: %s\n", name);
	int padre = 0;
	int hijo = dame_inodo_de_este_path(name, &el_fs, 0, &padre);
	printf("[INFO] padre = %i, hijo = %i\n", padre, hijo);

	if (el_fs.inodos[padre].modo == (__S_IFDIR | 0664)) {
		crear_directorio(padre, inodo_libre(&el_fs), 
		&name[ultimo_segmento_de_path(name)], &el_fs, __S_IFDIR);
		return 0;
	}
	return -1;
}

// Inicializa el Filesystem, ya con el inodo raiz creado.
void* fisopfs_init(struct fuse_conn_info *conn) {
	printf("[debug] fisopfs_init \n");

	if (access(filedisk, F_OK) == 0) {
		printf("[INFO] %s existe\n", filedisk);
		FILE *fs = fopen(filedisk, "rb");
		vsfs_t buffer;
		fread(&el_fs, sizeof(buffer), 1, fs);
		fclose(fs);
	} else {
		printf("[INFO] %s NO existe\n", filedisk);
		// crear un fs con solamente la raiz
		crear_raiz(&el_fs);
	}
}

// Crea un archivo en un directorio determinado. En la consola se utiliza el
// comando touch.
int fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	printf("[debug] fisopfs_create - path: %s\n", path);
	int padre = 0;
	int hijo = dame_inodo_de_este_path(path, &el_fs, 0, &padre);
	printf("[INFO] padre = %i, hijo = %i\n", padre, hijo);

	if (el_fs.inodos[padre].modo == (__S_IFDIR | 0664)) {  // ver esto del modo
		crear_archivo(padre,
		              inodo_libre(&el_fs),
		              &path[ultimo_segmento_de_path(path)],
		              NULL,
		              &el_fs,
		              __S_IFREG);
		return 0;
	}
	return -1;
}

// Remueve un archivo determinado. En la consola se utiliza el
// comando unlink.
int fisopfs_unlink(const char *path) {
	printf("[debug] fisopfs_unlink - path: %s\n", path);

	int padre = 0;
	int indice_inodo = dame_inodo_de_este_path(path, &el_fs, 0, &padre);
	if (indice_inodo == -1) {
		return -1;
	}
	if (el_fs.inodos[indice_inodo].modo == (__S_IFDIR | 0664)) {
		return -1;
	}

	el_fs.inodos[indice_inodo].links = 0;
	eliminar_inodo(&el_fs, indice_inodo);
	return 0;
}

// Se encarga de realizar la persistencia del Filesystem.
void fisopfs_destroy(void *arg) {
	printf("[debug] fisopfs_destroy \n");
	FILE *fs = fopen(filedisk, "wb");
	fwrite(&el_fs, sizeof(vsfs_t), 1, fs);
	fclose(fs);
}

// Abre el archivo en un determinado path, en donde retorna:
// -ENOENT en caso de que no exite el inodo del tipo regular.
// 0 en caso de exito.
int fisopfs_open(const char *path, struct fuse_file_info *fi) {
	printf("[debug] fisopfs_open - path: %s  \n", path);
	int padre = 0;
	int inodo = dame_inodo_de_este_path(path, &el_fs, 0, &padre);
	if (inodo < 0) {
		return -ENOENT;
	}
	inodo_actualizar_fecha_acceso(&el_fs.inodos[inodo], ahora());
	return 0;
}

// Trunca los archivos para reducir su tamaño (a size), donde retorna:
// -ENOENT en caso de que no exista el inodo de tipo regular.
// 0 en caso de exito.
// Se utliza el comando truncate.
int fisopfs_truncate(const char *path, off_t size) {
	printf("[debug] fisopfs_truncate - path: %s - size: %li \n", path, size);
	int ultimo_padre;
	int inodo = dame_inodo_de_este_path(path, &el_fs, 0, &ultimo_padre);
	if (inodo < 0) {
		return -ENOENT;
	}
	el_fs.inodos[inodo].size = size;
	inodo_actualizar_fecha_cambio(&el_fs.inodos[inodo], ahora());
	inodo_actualizar_fecha_acceso(&el_fs.inodos[inodo], ahora());
	inodo_actualizar_fecha_modificacion(&el_fs.inodos[inodo], ahora());
	return 0;
}

// Escribe en un arhivo existente y actualiza parte de la metada del archivo.
// Retorna el nuevo size en caso de exito.
int fisopfs_write(const char *path, const char *buf, 
size_t size, off_t offset, struct fuse_file_info *fi) {
	
	printf("[debug] fisopfs_write - path: %s - size: %li \n", path, size);
	// encontrar el archivesco
	int padre = 0;
	int inodo = dame_inodo_de_este_path(path, &el_fs, 0, &padre);

	char *contenido = (char *) &el_fs.bloques_datos[inodo][0];
	memcpy(&contenido[offset], buf, size);
	int size_anterior = el_fs.inodos[inodo].size;

	if (size_anterior < size_anterior + size) {
		el_fs.inodos[inodo].size = size_anterior + size;
	} else {
		el_fs.inodos[inodo].size = size;
	}
	// actualizar metadata
	inodo_actualizar_fecha_cambio(&el_fs.inodos[inodo], ahora());
	
	return size;
}

// Elimina un directorio determinado, retornando los sigueintes valores:
// -EPERM en caso de que se intente borrar el direcotrio Raiz.
// -ENOENT en caso de no existir el inodo del tipo directorio.
// -ENOTDIR en caso de no contar con los permisos para eliminar el direcotrio.
// -ENOTEMPTY en caso de eliminar un direcotrio que no esta vacio.
//  0 en caso de exito.
int fisopfs_rmdir(const char *path) {
	printf("[debug] fisopfs_rmdir - path: %s\n", path);

	// si es la raiz, no se borra
	if (strcmp(path, "/") == 0) {
		return -EPERM;
	}

	int padre = 0;
	int indice_inodo = dame_inodo_de_este_path(path, &el_fs, 0, &padre);
	if (indice_inodo == -1) {
		return -ENOENT;
	}
	if (el_fs.inodos[indice_inodo].modo != (__S_IFDIR | 0664)) {
		return -ENOTDIR;
	}

	directorio_t *directorio =
	        &((directorio_t *) &el_fs.bloques_datos[indice_inodo])[0];

	// si tiene archivescos no se borra
	if (directorio->cantidad_entradas > 0) {
		return -ENOTEMPTY;
	}
	// borrarlo
	eliminar_inodo(&el_fs, indice_inodo);

	return 0;
}

static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,
	.mkdir = fisopfs_mkdir,
	.init = fisopfs_init,
	.create = fisopfs_create,
	.unlink = fisopfs_unlink,
	.destroy = fisopfs_destroy,
	.write = fisopfs_write,
	.open = fisopfs_open,
	.truncate = fisopfs_truncate,
	.rmdir = fisopfs_rmdir,
	.utimens = fisopfs_utimens,
};

int main(int argc, char *argv[]) {
	for (int i = 1; i < argc - 1; i++) {
		if (strcmp(argv[i], "--filedisk") == 0) {
			filedisk = argv[i + 1];

			for (int j = i; j < argc - 1; j++) {
				argv[j] = argv[j + 2];
			}

			argc = argc - 2;
			break;
		}
	}

	return fuse_main(argc, argv, &operations, NULL);
}