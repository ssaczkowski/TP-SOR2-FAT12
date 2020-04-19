

#include <stdio.h>

#include <stdlib.h>



// Definimos la estructura de las tablas de particion, cada una de 16 bytes

typedef struct 

{

    unsigned char first_byte; // 1 byte

    unsigned char start_chs[3]; // 3 byte

    unsigned char partition_type; // 1 byte

    unsigned char end_chs[3]; // 3 byte

    char start_sector[4]; // 4 byte

    char length_sectors[4]; // 4 byte

} __attribute((packed)) PartitionTable; // Total = 16 bytes



// Definimos la estructura del Boot Sector. de tamaño 512 bytes

typedef struct 

{

    unsigned char jmp[3]; // 3 bytes

    char oem[8]; // 8 bytes

    unsigned short sector_size; // 2 bytes

    unsigned char sectors_per_cluster; // 1 byte

    unsigned short number_of_reserved_sectors; // 2 bytes

    unsigned char number_of_fats; // 1 byte

    unsigned short root_dir_entries; // 2 bytes

    unsigned short num_sectors; // 2 bytes. Si 2B(posicion de este campo) no es lo suficientemente grande, ajústelo a 0 y use el valor 4 B en los bytes 32-35 a continuación

    unsigned char media_type; // 1 byte. (0xf0 = disco extraíble, 0xf8 = disco fijo).

    unsigned short size_fat_in_sectors; // 2 bytes

    unsigned short sectors_per_track; // 2 bytes

    unsigned short num_headers; // 2 bytes

    unsigned int count_of_hidden_sectors; // 4 bytes 

    unsigned int total_logical_sectors; // 4 bytes. Este campo será 0 si el campo 2B anterior (bytes 19-20) no es cero.

    unsigned char num_physical_drive; // 1 byte.

    unsigned char ignore; // 1 byte

    unsigned char boot_signature; // 1 byte. Firma de arranque extendida para validar los siguientes tres campos (0x29)

    

    unsigned int volume_id;

    //char volume_id[4];

    char volume_label[11];

    char fs_type[8]; // Type en ascii

    char boot_code[448]; // Dentro de el estaria lo que es 0PartitionTable

    unsigned short boot_sector_signature; // 2 bytes

} __attribute((packed)) Fat12BootSector;



int main() 

{

    FILE * in = fopen("test.img", "rb");

    int i;

    //decraracion de 4 elementos de la estructura PartitionTable, que llamaremos pt.

    PartitionTable pt[4];

    //decraracion de un elemento de la estructura BootSector, que llamaremos bs.

    Fat12BootSector bs;

    //El desplazamiento es de SEEK_SET a 1BE hexa(446 en decimal) que es donde comienza las tablas de paticiones.    

    fseek(in, 0x1BE , SEEK_SET); // Ir al inicio de la tabla de particiones. Completar ...

    //fread(void *ptr, size_t size, size_t nmemb, FILE *stream)

    //*ptr=puntero a un bloque de memoria.

    //size=tamaño en bytes de cada elemento a leer.

    //nmenb=cantidad de elementos a leer (cada uno con un tamaño fijo (puesto en size)).

    //*stream=puntero al archivo a leer.

    fread(pt, sizeof(PartitionTable), 4, in); // leo entradas.

    

    printf("******************************************************\n");

    

    //Mostramos los datos de la estructura PartitionTable.

    for(i=0; i<4; i++) 

    {        

        printf("Tipo de Particion: %d\n", pt[i].partition_type);

        if(pt[i].partition_type == 1) 

        {

            printf("Encontrado FAT12 %d\n", i);

            break;

        }

    }

    

    //Checkeo de lectura correcta del File System.

    if(i == 4) 

    {

        printf("No se encontró filesystem FAT12, saliendo ...\n");

        return -1;

    }

    

    //Nos posicionamos al inicio del archivo para leer el Boot Sector.

    fseek(in, 0, SEEK_SET);

    //Leemos un elemento y lo estructuramos a la variable bs.

    fread(&bs, sizeof(Fat12BootSector), 1, in);

    

    printf("******************************************************\n");

    //Imprimimos los datos del Boot Sector.

    printf("* Jump code: %02X:%02X:%02X\n", bs.jmp[0], bs.jmp[1], bs.jmp[2]);

    printf("* OEM code: [%.8s]\n", bs.oem);

    printf("* Sector size: %d\n", bs.sector_size);

    printf("* Sectors per cluster: %hhu\n", bs.sectors_per_cluster);

    printf("* Number of reserved sectors: %d\n", bs.number_of_reserved_sectors);

    printf("* Number of Fats: %hhu\n", bs.number_of_fats);

    printf("* Root directory entries: %d\n", bs.root_dir_entries);

    printf("* Num sectors: %d\n", bs.num_sectors);

    printf("* Media type: %hhu\n", bs.media_type);

    printf("* Size fat in sectors: %d\n", bs.size_fat_in_sectors);

    printf("* Sectors per track: %d\n", bs.sectors_per_track);

    printf("* Number of headers: %d\n", bs.num_headers);

    printf("* Count of hidden sectors: %u\n", bs.count_of_hidden_sectors);

    printf("* Total logical sectors: %u\n", bs.total_logical_sectors);

    printf("* Number physical drive: %hhu\n", bs.num_physical_drive);

    printf("* Ignore: %hhu\n", bs.ignore);

    printf("* Boot signature: %hhu\n", bs.boot_signature);

    printf("* Volume id: %u\n", bs.volume_id);

    printf("* Volume label: [%.11s]\n", bs.volume_label);

    printf("* Filesystem type: [%.8s]\n", bs.fs_type);

    printf("* Boot sector signature: 0x%04X\n", bs.boot_sector_signature);

    printf("******************************************************\n");

    fclose(in);

    return 0;

}
