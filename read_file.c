
#include <stdio.h>
#include <stdlib.h>

// Definimos la estructura de las tablas de particion, cada una de 16 bytes
typedef struct 
{
    unsigned char first_byte;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    unsigned short starting_cluster;
    unsigned int file_size;
} __attribute((packed)) PartitionTable;

// Definimos la estructura del Boot Sector. de tamaño 512 bytes
typedef struct 
{
    unsigned char jmp[3];
    char oem[8];
    unsigned short sector_size;
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

    char volume_id[4];
    char volume_label[11];
    char fs_type[8];
    char boot_code[448];
    unsigned short boot_sector_signature;
} __attribute((packed)) Fat12BootSector;


// Definimos la estructura del Directory entry. 32 bytes cada uno.
typedef struct 
{
    unsigned char filename[1]; // IMPORTANTE : Primer carácter del nombre de archivo (ASCII) o estado de asignación (0x00 sin asignar, 0xE5 eliminado) 
    unsigned char name[7];
    unsigned char extension[3];
    unsigned char attributes[1]; // IMPORTANTE : 0x10 -> Es un directorio. 0x20 -> Es un archivo.
    unsigned char reserved[1];
    unsigned char created_time_in_seconds[1];
    unsigned short created_time_in_hours_minutes_seconds; // 2 bytes
    unsigned short created_day; // 2 bytes
    unsigned short accessed_day; // 2 bytes
    unsigned short high_first_cluster_address; // 2 bytes (es 0 tanto para FAT12 como FAT16)
    unsigned short written_time_in_hours_minutes_seconds; // 2 bytes
    unsigned short written_day; // 2 bytes
    unsigned short low_first_cluster_address; // 2 bytes
    unsigned int size_of_file; // 4 bytes

} __attribute((packed)) Fat12Entry;

void read_file(unsigned short low_first_cluster_address, int size_of_file, unsigned short first_cluster, unsigned short size_cluster)
{
    FILE * in = fopen("test.img", "rb");
    int i;
    char leer[size_of_file];
    
    fseek(in, first_cluster + ((low_first_cluster_address - 2) * size_cluster) , SEEK_SET);
    fread(leer, size_of_file, 1, in);
    
    for(i=0; i<size_of_file; i++) 
    {
        printf("%c", leer[i]);
    }
    
    fclose(in);
}

void print_file_info(Fat12Entry *entry, unsigned short first_cluster, unsigned short size_cluster) 
{
    switch(entry->filename[0]) 
    {
        case 0x00:
            return; // unused entry
        case 0xE5:
            return;
        default:
            switch(entry->attributes[0])
            {
                case 0x10:
                    //printf("Directorio : [%.1s%.7s.%.3s]\n", entry->filename, entry->name, entry->extension);
                    return;
                case 0x20:
                    printf("Archivo : [%.1s%.7s.%.3s]\n", entry->filename, entry->name, entry->extension);
                    read_file(entry->low_first_cluster_address, entry->size_of_file, first_cluster, size_cluster);
                    return;
                default:
                    return;
            }
    }
}

int main() 
{
    FILE * in = fopen("test.img", "rb");
    int i;
    PartitionTable pt[4];
    Fat12BootSector bs;
    Fat12Entry entry;

    unsigned short first_cluster;
    unsigned short size_cluster;

    fseek(in, 0x1BE , SEEK_SET); // Voy al inicio de la tabla de particiones.
    fread(pt, sizeof(PartitionTable), 4, in);

    printf("******************************************************\n");
    
    for(i=0; i<4; i++) 
    {        
        if(pt[i].partition_type == 1) 
        {
            printf("Encontrada particion FAT12 %d\n", i);
            break;
        }
    }
    
    if(i == 4) 
    {
        printf("No encontrado filesystem FAT12, saliendo...\n");
        return -1;
    }
    
    fseek(in, 0, SEEK_SET);
    //Leemos un elemento y lo estructuramos a la variable bs.
    fread(&bs, sizeof(Fat12BootSector), 1, in);

    //ftell=devuelve la posicion actual en el archivo.
    printf("En  0x%lX, sector size %d, FAT size %d sectors, %d FATs\n", 
           ftell(in), bs.sector_size, bs.size_fat_in_sectors, bs.number_of_fats);

    printf("******************************************************\n");

    fseek(in, (bs.number_of_reserved_sectors-1 + bs.size_fat_in_sectors * bs.number_of_fats) *
          bs.sector_size, SEEK_CUR);
    
    //obtenemos la direccion del primer cluster.
    first_cluster = ftell(in) + (bs.root_dir_entries * sizeof(entry));
    //sacamos el tamaño del cluster.
    size_cluster = bs.sector_size * bs.sectors_per_cluster;

    printf("Root dir_entries %d \n", bs.root_dir_entries);
    //Guarda en la estructura entry las 512 entradas de directorio, e imprime el tipo y nombre de los archivos.
    for(i=0; i<bs.root_dir_entries; i++) 
    {
        fread(&entry, sizeof(entry), 1, in);
        print_file_info(&entry, first_cluster, size_cluster);
    }
    //Posicion final 2560+(512*32)=18944=0x4A00.
    printf("\nLeido Root directory, ahora en 0x%lX\n", ftell(in));
    printf("******************************************************\n");
    
    fclose(in);
    return 0;
}