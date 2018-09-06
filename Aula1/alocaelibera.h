#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define MEMORY_SIZE 4096

typedef struct aloc {
    int size;
    uint8_t *p;
    struct aloc *prev;
    struct aloc *next;
} ALOC;

uint8_t memory[MEMORY_SIZE];
ALOC *first;

void *aloca(unsigned int bytes);
void libera(void *p);
ALOC *initAloc(unsigned int bytes, int offset, ALOC *prev, ALOC *next);

int main(void) {

    uint8_t* p = aloca(30);
    uint8_t* q = aloca(20);
    uint8_t* r = aloca(5);
    uint8_t* s = aloca(10);

    libera(p);

    uint8_t* a = aloca(2);
    uint8_t* b = aloca(8);

    libera(s);

    uint8_t* t = aloca(30);

    libera(q);

    uint8_t* u = aloca(4);
    uint8_t* v = aloca(3);

    uint8_t* w = aloca(14);

    return 0;

}

void *aloca(unsigned int bytes) {

    // Se memC3ria vazia
    if(!first) {
        // Verifica se o espaC'o requerido C) menor que o tamanho da memC3ria
        if (bytes < sizeof(memory)) {
            printf("Primeira alocaC'C#o: %d bytes\n", bytes);
            first = initAloc(bytes, 0, NULL, NULL);
            if (first->size == bytes) printf("Sucesso! EndereC'o: %d\n", first->p);
            return first->p; // Retorna ponteiro para espaC'o da memC3ria nC#o alocado
        }
        else {
            printf("ERROR: cannot alocate %d bytes. Not enough space!\n", bytes);
            return NULL;
        }
    }
    // Se memC3ria nC#o vazia
    else {
        // Buraco no comeC'o da memC3ria
        if (first->p - &memory[0] >= bytes) {
            printf("Buraco no comeC'o da memC3ria. Tamanho: %d bytes\n", first->p - &memory[0]);
            ALOC *aux = initAloc(bytes, 0, NULL, first);
            first->prev = aux;
            first = aux;
            if (first->size == bytes) printf("Sucesso! EndereC'o: %d\n", first->p);
            return first->p;
        }

        ALOC *aloc = first, *next = NULL;
        int offSet = aloc->size; // Deslocamento inicial de bytes
        printf("Outra alocaC'C#o: %d bytes. Offset inicial: %d\n", bytes, offSet);

        // Procurando endereC'o disponC-vel para a alocaC'C#o
        while (aloc->next != NULL) {
            printf("Procurando endereC'o disponC-vel...\n");
            // Procurando por "buracos" na memC3ria
            int availableSize = aloc->next->p - aloc->p - aloc->size;
            if (availableSize >= bytes) {
                printf("Buraco na memC3ria disponC-vel. Tamanho: %d bytes\n", aloc->next->p - aloc->p - aloc->size);
                next = aloc->next;
                break;
            }
            else if(availableSize > 0) {
                offSet += availableSize; // Incrementa offset com buracos na memC3ria
            }

            aloc = aloc->next;
            offSet += aloc->size; // Atualizando deslocamento de bytes
            printf("Offset incrementado. Novo valor: %d\n", offSet);
        }

        // Verificando espaC'o no final da memC3ria
        if (next == NULL && bytes > (&memory[sizeof(memory) - 1] - (aloc->p + aloc->size))) {
            printf("ERROR: cannot alocate %d bytes. Not enough space!\n", bytes);
            return NULL;
        }

        printf("Alocando depois do bloco de %d bytes\n", aloc->size);

        ALOC *nextAloc = initAloc(bytes, offSet, aloc, next); // Ponteiro para espaC'o da memC3ria nC#o alocada
        aloc->next = nextAloc; // Atualiza mapa de controle com o C:ltimo ponteiro
        if (aloc->next->size == bytes) printf("Sucesso! EndereC'o: %d\n", aloc->next->p);
        return nextAloc->p;
    }

}

void libera(void *p) {

    if (p != NULL && first != NULL) {
        // EspaC'o para ser desalocado corresponde ao primeiro ponteiro
        if (first->p == p) {
            printf("Desalocando primeiro bloco de %d bytes\n", first->size);
            first = first->next;
            first->prev = NULL;
        }
        // Procurando por ponteiro correspondente
        else {
            ALOC *aloc = first->next;
            bool control = false;
            while (!control) {
                printf("Procurando por bloco para desalocamento...\n");
                // Caso de parada do loop
                if (aloc->p == p) {
                    printf("Bloco encontrado...\n");
                    control = true;
                    break;
                }

                // Ponteiro p nC#o encontrado
                if (aloc->next == NULL) {
                    printf("ERROR: cannot free, p not found.");
                    break;
                }

                aloc = aloc->next;
            }

            // Ajustando mapa de controle
            if (control) {
                printf("Desalocando bloco de %d bytes\n", aloc->size);
                aloc->prev->next = aloc->next;
                if (aloc->next) aloc->next->prev = aloc->prev;
            }
        }
    }
    else {
        printf("ERROR: cannot free, p is null or memory is empty.");
    }

}

// Inicializa alocaC'C#o do mapa de controle
ALOC *initAloc(unsigned int bytes, int offset, ALOC *prev, ALOC *next) {

    ALOC *aloc = malloc(sizeof(ALOC));
    aloc->size = bytes;
    aloc->p = &memory[offset];
    aloc->prev = prev;
    aloc->next = next;

    if (next) next->prev = aloc; // Atualizando ponteiro anterior do prC3ximo

    return aloc;

}