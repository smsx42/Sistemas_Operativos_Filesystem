# fisop-fs

## Introito
A grandes rasgos implementamos algo similar al _vsfs_ explicado en _Operating Systems: Three Easy Pieces_. 

Tenemos un sector de máscara de _inodos_ para poder ubicar los bloques libres y un sector de datos dividido en bloques. El bloque puede contener o un archivo o un directorio.

## Generalidades

- Nuestro _filesystem_ se implementa con un arreglo de bloques de 4K.
- Cada archivo ocupa un bloque. 
- Cada archivo puede ser un archivo regular o un directorio.
- Hay un número limitado y fijo de _inodos_ y bloques.
- Un _inodo_ es una estructura que contiene metadata de un archivo.
- Cada bloque se corresponde a un _inodo_ del siguiente modo:
    - El primer inodo (0) se corresponde con el primer bloque (0)
    - En el primer bloque se encuentra el directorio raíz.
    - El inodo _n_ corresponde al bloque _n_.
- Un directorio es una estructura que contiene un arreglo de _dirents_
- Un _dirent_ es una estructura que contiene un número de _inodo_ y un nombre.

Entonces un directorio guarda nombres de archivos con sus respectivos números de _inodo_.

## Esquema de las estructuras del fs

![esquema-de-estructuras](https://drive.google.com/uc?export=view&id=1aVAsYSkw_t1s4QK95qZpDlARdy14ryhF)

## Búsqueda de archivo en _path_

Se comienza desde la raíz (bloque 0, entrada 0) y se descompone el _path_ sucesivamente hasta hallar su _inodo_. Si el archivo no existiese se retorna -1.

Para facilitar la creación de archivos al buscar un archivo en el _path_ también se obtiene el "padre" del archivo en cuestion.

## Persistencia

Al montar el _filesystem_ en la función `fisopfs_init` se verifica la existencia de `filedisk`. En caso de existir se lo lee y carga en memoria.

Al momento de desmontar el _filesystem_ en la función `fisopfs_destroy` se sobreescribe el archivo `filedisk`.

# Pruebas

## Montar el fs

### Comandos:

```sh
mkdir prueba
./fisopfs -f prueba 
mount | grep fisopfs
```

### Capturas:

![montaje-1](documentacion/img/capturas/montaje-1.png)
![montaje-2](documentacion/img/capturas/montaje-2.png)


## Crear directorios (un nivel)

### Comandos:

```sh
mkdir prueba
./fisopfs -f prueba
mkdir prueba/nuevo-dir-nivel-1
ls -la prueba
ls -la prueba/nuevo-dir-nivel-1
stat prueba/nuevo-dir-nivel-1
```

### Capturas:

![directorio-crear-1-nivel_1](documentacion/img/capturas/directorio-crear-1-nivel_1.png)
![directorio-crear-1-nivel_2](documentacion/img/capturas/directorio-crear-1-nivel_2.png)

## Crear directorios (dos niveles)

### Comandos:

```sh
mkdir prueba
./fisopfs -f prueba
mkdir prueba/nuevo-dir-nivel-1
mkdir prueba/nuevo-dir-nivel-1/nuevo-dir-nivel-2
ls -la prueba/nuevo-dir-nivel-1
ls -la prueba/nuevo-dir-nivel-1/nuevo-dir-nivel-2
stat prueba/nuevo-dir-nivel-1
stat prueba/nuevo-dir-nivel-1/nuevo-dir-nivel-2
```

### Capturas:

![directorio-crear-2-niveles_1](documentacion/img/capturas/directorio-crear-2-niveles_1.png)
![directorio-crear-2-niveles_2](documentacion/img/capturas/directorio-crear-2-niveles_2.png)

## Directorios - borrar vacío

### Comandos:

```sh
mkdir prueba
./fisopfs -f prueba
mkdir prueba/borrame
ls -la prueba
ls -la prueba/borrame
rmdir prueba/borrame
ls -la prueba
```

### Capturas:
![directorio-borrar-vacio](documentacion/img/capturas/directorio-borrar-vacio.png)

## Directorios - borrar con contenido (`rmdir`, `rm -r`)

### Comandos:

```sh
mkdir prueba
./fisopfs -f prueba
mkdir prueba/borrame
echo "hola" > prueba/borrame/archivo
ls -la prueba
ls -la prueba/borrame
rmdir prueba/borrame
ls -la prueba
cat prueba/borrame/archivo
rm -r prueba/borrame
ls -la prueba
```

### Capturas:
![directorio-borrar-con-contenido](documentacion/img/capturas/directorio-borrar-con-contenido.png)


## Archivos - crear archivo vacío

### Comandos:

```sh
mkdir prueba
./fisopfs -f prueba
ls -la prueba
touch prueba/archivo
ls -la prueba
ls -la prueba/archivo
stat prueba/archivo
# limpiar
rmdir prueba
rm persistence_file.fisopfs
```

### Capturas:

![archivos-crear-archivo-vacio-1](documentacion/img/capturas/archivos-crear-archivo-vacio-1.png)
![archivos-crear-archivo-vacio-2](documentacion/img/capturas/archivos-crear-archivo-vacio-2.png)

## Archivos - crear archivo con contenido (`echo`) - leer archivo (`cat`)

### Comandos:

```sh
mkdir prueba
./fisopfs -f prueba
ls -la prueba
echo "soy un archivo" > prueba/archivo
ls -la prueba/archivo
cat prueba/archivo
stat prueba/archivo

```

### Capturas:

![archivos-crear-archivo-con-contenido-2](documentacion/img/capturas/archivos-crear-archivo-con-contenido-2.png)


## Archivos - leer archivo (`more`, `less`, `vim`)

### Comandos:

```sh
more prueba/archivo
less prueba/archivo
vim prueba/archivo
```

### Capturas:

![archivos-leer-archivo-more](documentacion/img/capturas/archivos-leer-archivo-more.png)
![archivos-leer-archivo-less](documentacion/img/capturas/archivos-leer-archivo-less.png)
![archivos-leer-archivo-vim](documentacion/img/capturas/archivos-leer-archivo-vim.png)


## Archivos - sobreescribir (truncar)

### Comandos:

```sh
mkdir prueba
./fisopfs -f prueba
echo "soy un archivo" > prueba/archivo
cat prueba/archivo
echo "nuevo contenido" > prueba/archivo
cat prueba/archivo
```

### Capturas:

![archivos-sobreescribir-truncar](documentacion/img/capturas/archivos-sobreescribir-archivo-truncar.png)


## Archivos - sobreescribir (_append_)

### Comandos:

```sh
mkdir prueba
./fisopfs -f prueba
echo "soy un archivo" > prueba/archivo
cat prueba/archivo
echo "nuevo contenido" >> prueba/archivo
cat prueba/archivo
```

### Capturas:

![archivos-sobreescribir-append](documentacion/img/capturas/archivos-sobreescribir-append.png)


## Archivos - borrar (`rm`)

### Comandos:

```sh
mkdir prueba
./fisopfs -f prueba
echo "soy un archivo" > prueba/archivo
cat prueba/archivo
ls -la prueba
rm prueba/archivo
ls -la prueba
```

### Capturas:

![archivos-borrar-archivo](documentacion/img/capturas/archivos-borrar-archivo.png)



## Persistencia (_filedisk_ _default_)

Usar el _filedisk_ por defecto. 



### Comandos:

```sh
# Parte 1. Montar por primera vez.
# mostrar que fs no existe
ls
mkdir prueba
./fisopfs -f prueba 
ls -la prueba
stat prueba
# crear directorio y archivesco
mkdir prueba/persistencia
echo "soy un archivo persistente" > prueba/archivo
echo "yo tambien soy un archivo persistente" > prueba/persistencia/otro-archivo
ls -la prueba
ls -la prueba/persistencia
# terminar el proceso fisopfs con ctrl c y mostrar que el fs existe
ls -la 

# Parte 2. Volver a montarlo
./fisopfs -f prueba
ls -la prueba
cat prueba/archivo
cat prueba/persistencia/otro-archivo

```

### Capturas:

Antes de montar:
![persistencia-default](documentacion/img/capturas/persistencia-default-1.png)

Montamos y creamos archivos:
![persistencia-default](documentacion/img/capturas/persistencia-default-2.png)

Desmontamos y se crea el archivo de persistencia (`persistence_file.fisopfs`)
![persistencia-default](documentacion/img/capturas/persistencia-default-3.png)

Volvemos a montarlo:
![persistencia-default](documentacion/img/capturas/persistencia-default-4.png)

Comprobamos que los archivos existen:
![persistencia-default](documentacion/img/capturas/persistencia-default-5.png)


## Persistencia (_filedisk_ provisto)

Usar el _filedisk_ que se pasa como argumento. 

### Comandos:

```sh
# Parte 1. Montar por primera vez.
# mostrar que fs no existe
ls
mkdir prueba
./fisopfs -f --filedisk persistencia-personalizada prueba
ls -la prueba
stat prueba
# crear directorio y archivesco
mkdir prueba/persistencia
echo "soy un archivo persistente" > prueba/archivo
echo "yo tambien soy un archivo persistente" > prueba/persistencia/otro-archivo
ls -la prueba
ls -la prueba/persistencia
# terminar el proceso fisopfs con ctrl c y mostrar que el fs existe
ls -la 

# Parte 2. Volver a montarlo
./fisopfs -f --filedisk persistencia-personalizada prueba
ls -la prueba
cat prueba/archivo
cat prueba/persistencia/otro-archivo

```

### Capturas:

Antes de montar:
![persistencia-personalizada](documentacion/img/capturas/persistencia-personalizada-1.png)

Montamos y creamos archivos:
![persistencia-personalizada](documentacion/img/capturas/persistencia-personalizada-2.png)

Desmontamos y se crea el archivo de persistencia (`persistencia-personalizada`)
![persistencia-personalizada](documentacion/img/capturas/persistencia-personalizada-3.png)

Volvemos a montarlo:
![persistencia-personalizada](documentacion/img/capturas/persistencia-personalizada-4.png)

Comprobamos que los archivos existen:
![persistencia-personalizada](documentacion/img/capturas/persistencia-personalizada-5.png)

