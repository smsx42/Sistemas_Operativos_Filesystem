# solamente los comandos necesarios suponiendo raíz = prueba
# se prueba cada caso por separado, es decir
# cada vez se crean al comenzar y se borrar al finalizar 
# tanto prueba como el filedisk.


# FILESYSTEM
#[funcionalidad] montar fs
# montar y chequear que existe el fs
mkdir prueba
./fisopfs -f prueba 
mount | grep fisopfs
# ctrl c  fisopfs
rm -r prueba
rm persistence_file.fisopfs

# ARCHIVESCOS
#[funcionalidad] crear archivo
# crear archivo vacío
mkdir prueba
./fisopfs -f prueba
ls -la prueba
touch prueba/archivo
ls -la prueba
ls -la prueba/archivo
stat prueba/archivo
rmdir prueba
rm persistence_file.fisopfs

#[funcionalidad] escribir archivo
# escribir al crear con echo
mkdir prueba
./fisopfs -f prueba
ls -la prueba
echo "soy un archivo" > prueba/archivo
ls -la prueba/archivo
cat prueba/archivo
stat prueba/archivo

#[funcionalidad] leer archivo (despues de crear con echo)
mkdir prueba
./fisopfs -f prueba
ls -la prueba
echo "soy un archivo" > prueba/archivo
more prueba/archivo
less prueba/archivo
vim prueba/archivo

#[funcionalidad] sobreescribir archivo truncar
mkdir prueba
./fisopfs -f prueba
echo "soy un archivo" > prueba/archivo
cat prueba/archivo
echo "nuevo contenido" > prueba/archivo
cat prueba/archivo
#
rmdir prueba
rm persistence_file.fisopfs

#[funcionalidad] sobreescribir archivo append
mkdir prueba
./fisopfs -f prueba
echo "soy un archivo" > prueba/archivo
cat prueba/archivo
echo "nuevo contenido" >> prueba/archivo
cat prueba/archivo

rm -r prueba
rm persistence_file.fisopfs

#[funcionalidad] borrar archivo
mkdir prueba
./fisopfs -f prueba
echo "soy un archivo" > prueba/archivo
cat prueba/archivo
ls -la prueba
rm prueba/archivo
ls -la prueba

# ESTADISTICAS DE ARCHIVOS
# hacer con alguno que cree archivo y directorio
# hacer stat, esperar un minuto y hacer stat de nuevo
#[funcionalidad] último acceso

# lo mismo que acceso pero modificando el archivesco
#[funcionalidad] última modificación

# DIRECTORIOS
#[funcionalidad] crear directorio

# crear directorio 1 nivel
mkdir prueba
./fisopfs -f prueba
mkdir prueba/nuevo-dir-nivel-1
ls -la prueba
ls -la prueba/nuevo-dir-nivel-1
stat prueba/nuevo-dir-nivel-1
rm -r prueba
rm persistence_file.fisopfs

# crear directorio 2 niveles
mkdir prueba
./fisopfs -f prueba
mkdir prueba/nuevo-dir-nivel-1
mkdir prueba/nuevo-dir-nivel-1/nuevo-dir-nivel-2
ls -la prueba/nuevo-dir-nivel-1
ls -la prueba/nuevo-dir-nivel-1/nuevo-dir-nivel-2
stat prueba/nuevo-dir-nivel-1
stat prueba/nuevo-dir-nivel-1/nuevo-dir-nivel-2
rm -r prueba
rm persistence_file.fisopfs


#[funcionalidad] borrar directorio

# nivel 1
mkdir prueba
./fisopfs -f prueba
mkdir prueba/borrame
ls -la prueba
ls -la prueba/borrame
rmdir prueba/borrame
ls -la prueba
rmdir prueba
rm persistence_file.fisopfs

# nivel 2
mkdir prueba
./fisopfs -f prueba
mkdir prueba/subir
mkdir prueba/subdir/borrame

ls -la prueba/subdir
ls -la prueba/subdir/borrarme
rm -r /prueba/subdir/borrame
ls -la prueba/subdir
rm -r prueba
rm persistence_file.fisopfs

# borrar directorio con archivos
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

#[funcionalidad] leer directorio (se hace en los otros test)

# PERSISTENCIA
# ojo que solo funca en modo debug (en discord se habla de eso [https://discord.com/channels/768876051518324749/1124534815115452546])
# vacío y levanta deben correrse en seguidilla para que surtan efecto

# usar filedisk default vacío (1 vez)
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

# usar filedisk default levanta (2 vez)
./fisopfs -f prueba
ls -la prueba
cat prueba/archivo
cat prueba/persistencia/otro-archivo

rm -r prueba
rm persistence_file.fisopfs

# lo mismo que lo anterior pero mostrando que el filedisk tien el nombre provisto
# usar filedisk default provisto (1 vez)
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

# usar filedisk default levanta (2 vez)
./fisopfs -f --filedisk persistencia-personalizada prueba
ls -la prueba
cat prueba/archivo
cat prueba/persistencia/otro-archivo

rm -r prueba
rm persistence_file.fisopfs