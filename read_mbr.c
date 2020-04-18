#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE * in = fopen("test.img", "rb");
    unsigned int i, start_sector, length_sectors;
    
    // Lectura de los primeros bytes, (MBR)
    
        if (in == NULL)
        {
            printf("\n¡ Error al abrir el archivo! \n\n");
        }
        else
        { 
	  
	    printf("\n  ******************** Primeros 446 bytes (gestor de arranque) ******************* \n\n");
	  
	        int firstBytes = 446;
	   
	        for(int j=0 ; j < firstBytes; j++) {
	      
	            i = fgetc(in);
          
	            printf("%02X", i);
		
	        }
	    
	   
	    printf("\n ******************************** Particiones: ******************************** \n\n");
    
    fseek(in, 0x1BE , SEEK_SET); // Voy al inicio.
    
    for(i=0; i<4; i++) { // Leo las entradas - Particiones -
        printf("Entrada de partición %d: Primer Byte %02X\n", i, fgetc(in));
        printf("  Comienzo de partición en CHS: %02X:%02X:%02X\n", fgetc(in), fgetc(in), fgetc(in));
        printf("  Tipo de partición 0x%02X\n", fgetc(in));
        printf("  Fin de partición en CHS: %02X:%02X:%02X\n", fgetc(in), fgetc(in), fgetc(in));
        
        fread(&start_sector, 4, 1, in);
        fread(&length_sectors, 4, 1, in);
        printf("  Dirección LBA relativa 0x%08X, de tamaño en sectores %d\n", start_sector, length_sectors);
        
        printf("\n\n");
    }
            
	    printf("\n ****************************************************************************** \n\n");
	}
    
    fclose(in);
    return 0;
}
