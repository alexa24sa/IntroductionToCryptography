#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // usadas para seed del PRNG fallback
#define MAX_SIZE 100


// Convierte una cadena HEX a un buffer de bytes
// Lee una cadena de caracteres hexadecimales (ej: "4a6f7365") y la convierte
// en bytes binarios (ej: [0x4a, 0x6f, 0x73, 0x65]).
int hex_a_bytes(const char *hex_str, unsigned char *buffer, size_t max_len) {
    size_t len = strlen(hex_str);
    if (len % 2 != 0 || len / 2 > max_len) {
        return -1; // Longitud incorrecta o excede el buffer
    }

    for (size_t i = 0; i < len / 2; i++) {
        // Lee dos caracteres hex y los convierte a un byte
        if (sscanf(&hex_str[i * 2], "%2hhx", &buffer[i]) != 1) {
            return -1;
        }
    }
    return len / 2; // Retorna la longitud en bytes
}


void generador_alt_key(unsigned char *key, size_t len) {
    // Nota: esto NO es criptográficamente seguro como libsodium, pero evita el error
    // cuando la librería no está disponible.
    for (size_t i = 0; i < len; i++) {
        key[i] = (unsigned char)(rand() & 0xFF);
    }
}


void cifrado(unsigned char *m, unsigned char *key, unsigned char *cifrado_out, size_t len) {
    for (size_t i = 0; i < len; i++) {
        cifrado_out[i] = m[i] ^ key[i];
    }
    // NOTA: Para descifrar, se usa la misma operacion: m[i] = cifrado_out[i] ^ key[i];
}

// Funcion auxiliar para imprimir el contenido de un buffer en hexadecimal.
void imprimir_buffer_hex(unsigned char *buffer, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", buffer[i]);
    }
    printf("\n");
}


int main()
{
    unsigned char cadena[MAX_SIZE]; // Asignado para mensaje, o entrada hex
    unsigned char cifrada[MAX_SIZE];
    size_t len = 0;
    int opcion;

    printf("\n\t*** CIFRADOR/DESCIFRADOR XOR *** \n");
    printf("Seleccione una opción:\n");
    printf("1. Cifrar Mensaje (Generar nueva llave)\n");
    printf("2. Descifrar Mensaje (Necesita llave)\n");
    printf("Opción: ");
    
    // Lee la opción
    if (scanf("%d", &opcion) != 1) {
        fprintf(stderr, "Entrada inválida.\n");
        return 1;
    }
    // Limpia el buffer de entrada después de scanf
    while (getchar() != '\n');
    
    // seed para el PRNG de fallback (necesario solo para cifrado, pero se inicializa aquí)
    srand((unsigned)time(NULL));

    // Puntero para la llave, se asignará memoria si es necesario
    unsigned char *key = NULL; 

    // OPCIÓN 1: CIFRADO
    if (opcion == 1) {
        printf("\n--- MODO CIFRADO ---\n");
        printf("Ingrese su cadena de texto a cifrar (long max. %d caracteres): ", MAX_SIZE - 1);
        
        // Lee la cadena
        if (fgets((char*)cadena, MAX_SIZE, stdin) == NULL) {
            fprintf(stderr, "Error leyendo la entrada.\n");
            return 1;
        }

        // Ajusta la longitud: elimina el '\n' y ajusta len.
        len = strlen((char *)cadena);
        if (len > 0 && cadena[len - 1] == '\n') {
            cadena[len - 1] = '\0';
            len--;
        }
        
        if (len == 0) {
            printf("No se ingresó mensaje válido.\n");
            return 0;
        }

        // Asignación de memoria para la llave del mismo tamaño que el mensaje
        key = (unsigned char *)malloc(len * sizeof(unsigned char)); 
        if (key == NULL) {
            fprintf(stderr, "Error de asignación de memoria.\n");
            return 1;
        }
        
        // Genera la llave
        generador_alt_key(key, len); 

        printf("\nEl mensaje (texto plano): %s\n", cadena);
        printf("La **LLAVE** necesaria para descifrar (Hex): ");
        imprimir_buffer_hex(key, len);

        // Cifra (cadena ^ key = cifrada)
        cifrado(cadena, key, cifrada, len);

        printf("\nEl mensaje cifrado (Hex): ");
        imprimir_buffer_hex(cifrada, len);


    // OPCIÓN 2: DESCIFRADO
   
    } else if (opcion == 2) {
        printf("\n--- MODO DESCIFRADO ---\n");
        char hex_input[MAX_SIZE * 2 + 1]; // Buffer para la entrada Hex (2 chars por byte + '\0')
        unsigned char cifrado_a_descifrar[MAX_SIZE];
        size_t len_hex_str;

        // 1. Pide la CADENA CIFRADA (en Hex)
        printf("Ingrese la cadena CIFRADA (en Hexadecimal, sin espacios): ");
        if (fgets(hex_input, MAX_SIZE * 2 + 1, stdin) == NULL) {
            fprintf(stderr, "Error leyendo la entrada cifrada.\n");
            return 1;
        }
        len_hex_str = strlen(hex_input);
        if (len_hex_str > 0 && hex_input[len_hex_str - 1] == '\n') {
            hex_input[len_hex_str - 1] = '\0'; // Quita el '\n'
            len_hex_str--;
        }

        // Convierte la cadena Hexadecimal cifrada a bytes
        len = hex_a_bytes(hex_input, cifrado_a_descifrar, MAX_SIZE);
        if (len <= 0) {
            fprintf(stderr, "Error: Cadena cifrada Hex inválida o vacía.\n");
            return 1;
        }
        
        // 2. Pide la LLAVE (en Hex)
        printf("Ingrese la LLAVE utilizada (en Hexadecimal, %zu bytes esperados): ", len);
        if (fgets(hex_input, MAX_SIZE * 2 + 1, stdin) == NULL) {
            fprintf(stderr, "Error leyendo la llave.\n");
            return 1;
        }
        len_hex_str = strlen(hex_input);
        if (len_hex_str > 0 && hex_input[len_hex_str - 1] == '\n') {
            hex_input[len_hex_str - 1] = '\0'; // Quita el '\n'
        }

        // Asigna memoria para la llave a descifrar
        key = (unsigned char *)malloc(len * sizeof(unsigned char)); 
        if (key == NULL) {
            fprintf(stderr, "Error de asignación de memoria.\n");
            return 1;
        }
        
        // Convierte la llave Hexadecimal a bytes
        if (hex_a_bytes(hex_input, key, len) != (int)len) {
            fprintf(stderr, "Error: La llave Hex es inválida o tiene longitud incorrecta.\n");
            free(key);
            return 1;
        }

        // Descifra (cifrado ^ key = mensaje original)
        // La función 'cifrado' realiza XOR y sirve para descifrar
        cifrado(cifrado_a_descifrar, key, cadena, len); 

        printf("\nEl mensaje DESCIFRADO (Texto plano): %s\n", cadena);

    } else {
        printf("Opción no reconocida.\n");
    }
    
    // 6. Liberamos la memoria dinámica si fue asignada
    if (key != NULL) {
        free(key); 
    }

    return 0;
}
