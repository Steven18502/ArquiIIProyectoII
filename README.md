# Proyecto 2 del curso de Arquitectura de Computadores II del primer semestre 2024 del Instituto Tecnológico de Costa Rica

## Cluster Beowulf para algoritmo de procesamiento de imágenes

### Instalación y Configuración del Cluster Beowulf


1. Instalación de paquetes básicos para el cluster en TODOS los nodos

Descargar la versión más estable de [Open MPI](https://www.open-mpi.org/software/)

Crear un directorio en $HOME donde instalarlo
```bash
mkdir $HOME/openmpi
```

Mover el archivo comprimido descargado al directorio recién hecho
```bash
mv $HOME/Downloads/openmpi-X.X.X.tar.gz $HOME/openmpi/
```

Ir al directorio creado por medio de la terminal
```bash
cd $HOME/openmpi/
```

Extraer el paquete
```bash
tar -xzvf openmpi-X.X.X.tar.gz
```

Ir al directorio donde se descomprime el paquete
```bash
cd $HOME/openmpi/openmpi-X.X.X
```

Instalar build-essential
```bash
sudo apt-get install build-essential
```

Configurar donde instalar Open MPI
```bash
./configure --prefix=$HOME/openmpi
```

Compilar el paquete de MPI
```bash
make all
```
Una vez compilado, se instala
```bash
make install
```

Instalar paquetes complementarios para MPI
```bash
sudo apt-get install openmpi-bin libopenmpi-dev
```

Agregar Open MPI a las variables de entorno
```bash
export PATH=$PATH:$HOME/openmpi/bin
```
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/openmpi/lib
```

Verificar la instalación con la versión del compilador de c en open mpi
```bash
mpicc -v
```


2. Configuración de red estática en el nodo esclavo

Si es necesario, crear una máquina virtual. En su configuración de red, escoger Bridged Adapter. No utilizar el default de 'NAT'

Poner el nombre del adaptador que se puede encontrar con el siguiente comando en el nodo maestro
```bash
ip address
```

Asegurarse de ingresar la misma dirección MAC que el adaptador

Abrir la máquina virtual desde el nodo maestro
```bash
sudo virt-manager
```
Tip: Configurar el sistema operativo con el mismo nombre de usuario en todas las máquinas (user)

Esta parte ya puede ser hecha en otra computadora

Ir a la parte de la red cableada en la configuración de redl (nodo esclavo)

En la parte de IPv4:

Asignar el método manual

**Asignar una Dirección:**

Dirección: 192.168.122.101

Máscara de red: 255.255.255.0

Puerta de enlace: 192.168.122.1

**Configurar el DNS:**

Desmarcar el automático

Asignar el 192.168.50.1

Desconectarse y volverse a conectar para que los cambios ocurran


3. Descarga de dependencias para la configuración del cluster

Instalar dependencias en el nodo esclavo
```bash
sudo apt-get install ssh
```
```bash
sudo apt-get install nfs-common portmap
```

Instalar dependencias en el nodo maestro
```bash
sudo apt-get install nfs-kernel-server nfs-common portmap
```


4. Configuración de red estática en el nodo maestro

Ir a la parte de la red cableada en la configuración de red

En la parte de IPv4:

Asignar el método manual

**Asignar una Dirección:**

Dirección: 192.168.50.100

Máscara de red: 255.255.255.0

Puerta de enlace: 192.168.50.1

**Configurar el DNS:**

Desmarcar el automático

Asignar el 192.168.50.1

Proceder a conectar el nodo maestro en la red cableada para que el LAN funcione


5. Configuración del cluster del lado del nodo maestro

Para que el maestro sea capaz de ejecutar comandos en cada uno de los nodos esclavos se necesita que estos últimos puedan tener acceso al ssh.

Se crea una llave para el ssh
```bash
ssh-keygen
```
no asignar file
dejar empty el passphrase

Se crea el key de forma aleatoria

Se procede a copiar la llave que se acaba de crear al nodo esclavo, por medio del protocolo SCP
```bash
scp .ssh/id_rsa.pub slaveUser@192.168.122.101:/home/slaveUser
```

Ahora en el nodo esclavo

Crear directorio .ssh para que el nodo maestro pueda obtener el key al conectarse
```bash
mkdir .ssh
```

Se le otorgan los permisos al folder
```bash
chmod 700 .ssh
```

Se mueve el archivo dentro de .ssh como "authorized_keys"
```bash
mv id_rsa.pub .ssh/authorized_keys
```

Si al correr el programa paralelizado da problemas la conexión intentar hacer este paso anterior solo con este comando, el usuario del esclavo y el ip del esclavo
```bash
ssh-copy-id slaveUser@192.168.122.101
```

Para acceder al nodo esclavo desde el nodo maestro

Se hace una conexión vía ssh
```bash
ssh slaveUser@192.168.122.101
```

Ahora se puede controlar la máquina a la cual se conectó.

Para terminar la conexión
```bash
exit
```

Se deben configurar así cada uno de los nodos.
Editar los archivos hosts
```bash
sudo nano /etc/hosts
```

Se abrirá el archivo para editar y se deben agregar las direcciones ip del master y los nodos
```txt
192.168.50.100 master master
192.168.122.101 node1 node1
192.168.50.102 node2 node2
192.168.122.103 node3 node3
```

Ahora se podrá conectar por ssh a partir del nombre guardado, pero con el usuario del nodo
```bash
ssh slaveUser@node1
```
Si tienen el mismo usuario, no es necesario ponerlo
```bash
ssh node2
```


6. Configuración del servidor de archivos NFS

En cada nodo del cluster, se necesita que todos los programas distribuidos por ejecutar tengan a disposición una carpeta compartida entre ellos. Esto será posible con la librería instalada anteriormente nfs-kernel-server.

Se configurará el recurso compartido

Se creará un directorio en el home del nodo maestro. Este debería quedar a la par del directorio con nombre del usuario en /home/cluster/
```bash
cd ..
sudo mkdir clusterdir
```

Como root se exportará el directorio para que se pueda acceder remotamente, a través del archivo export
```bash
sudo nano /etc/exports
```

Se abrirá para editar el archivo. Se debe agregar la ruta del directorio de la carpeta en el nodo maestro y las ips de los nodos esclavos
```txt
/home/clusterdir 192.168.122.101(rw,no_subtree_check,async,fsid=0,no_root_squash,insecure)
```

Se debe reiniciar el servicio de nfs
```bash
sudo /etc/init.d/nfs-kernel-server restart
```

Para verificar que se pueda acceder a la carpeta compartida desde el nodo esclavo se utiliza el comando showmount con el ip del nodo master
```bash
showmount -e 192.168.50.100
```

Se montará el recurso desde el cli para ser agregado a fstab. Así se hará el montado automático para que cada vez que arranque el nodo se cree en /home del nodo el directorio clusterdir igual que el nodo maestro. Ahí estarán los recursos compartidos del nodo maestro a través de nfs

Se creará el directorio en el nodo esclavo
```bash
sudo mkdir ../clusterdir
```

Se abre el archivo fstab para editarlo
```bash
sudo nano /etc/fstab
```

Se modifica el archivo fstab agregando al final el ip del nodo maestro, la dirección del directorio del nodo maestro, la dirección del directorio del nodo esclavo
```txt
192.168.50.100:/home/clusterdir /home/clusterdir nfs rw,sync,hard,intr 0 0
```

Se monta
```bash
sudo mount -a
```

Para verificar la conexión
```bash
mount
```

Si se desea desmontar (no hacer)
```bash
sudo umount -a
```

7. Configuración del entorno de desarrollo

El paquete de build-essentials tiene lo necesario para desarrollar y compilar, que ya se intenta instalar al inicio (usualmente ya viene en el SO)


8. Configuración para MPI

Se crea un archivo de compilación .mpi_hostfile en el nodo maestro

```bash
nano .mpi_hostfile
```

Se edita y se le agrega
```txt
#Nodo maestro
localhost slots=1

#Nodo esclavo1
node1 slots=1

#Nodo esclavo2
node2 slots=1

#Nodo esclavo3
node3 slots=1
```

Cada slot son la cantidad de procesos por ejecutar

Aquí se puede realizar ya una prueba del cluster

9. Compilación del código por paralelizar

El código por correr paralelamente debe estar dentro de la carpeta compartida junto a los archivos que necesite.

Debe incluir la libreria mpi y sus comandos para correrlo paralelamente

Se debe compilar con el compilador de mpi
Para este proyecto se codificó en el lenguaje c por lo que se usrá así
```bash
mpicc -o <executable_name> <program_code_file> -lm
```

Por ejemplo:
```bash
mpicc -o gaussian_blur_mpi gaussian_blur_mpi.c -lm
```


10. Ejecución del programa paralelizable con mpi

Su binario ejecutable debe estar dentro de la carpeta compartida por nfs

Para correrlo con mpi:
```bash
mpirun -np <number_of_processes> --hostfile <hostfile_name>./<executable_name> <upper_limit>
```

Por ejemplo:
```bash
mpirun -np 4 --hostfile .mpi_hostfile ./gaussian_blur_mpi
```


11. Para volver a correr el programa después de apagar las computadoras o si se pierde la conexión del mount nfs

Reiniciar el servicio de nfs en el nodo master
```bash
sudo /etc/init.d/nfs-kernel-server restart
```

Verificar que el nodo master siga compartiendo la carpeta por nfs en el nodo esclavo
```bash
showmount -e 192.168.50.100
```

Volver a hacer el mount en el nodo esclavo
```bash
sudo mount -a
```

Se recomienda salirse de la carpeta /home/clusterdir/ y volver a ingresar, si se encuentra en esta, para que los archivos carguen nuevamente

