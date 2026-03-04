#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define N 12        
#define ALTURA 15  // Aumentado para melhor visualização lateral
#define RAIO 10    // Ajustado para o tamanho do SRU

#define SRU_LARG 80
#define SRU_ALT  40

typedef struct {
    float x, y, z;
} Vertex;

typedef struct Face {
    int v[3];
    struct Face* opposite[3];
} Face;

int SRU[SRU_ALT][SRU_LARG];

void limparSRU() {
    for(int i = 0; i < SRU_ALT; i++)
        for(int j = 0; j < SRU_LARG; j++)
            SRU[i][j] = 0;
}

void plot(int x, int y) {
    if(x >= 0 && x < SRU_LARG && y >= 0 && y < SRU_ALT)
        SRU[y][x] = 1;
}

void imprimirSRU() {
    for(int i = 0; i < SRU_ALT; i++) {
        for(int j = 0; j < SRU_LARG; j++) {
            printf(SRU[i][j] ? "#" : " ");
        }
        printf("\n");
    }
}

void desenharLinha(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    while(1) {
        plot(x0, y0);
        if(x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if(e2 > -dy) { err -= dy; x0 += sx; }
        if(e2 < dx)  { err += dx; y0 += sy; }
    }
}

// Projeção Ortogonal Topo (Plano XY)
void projetarTopo(Vertex v, int* x2d, int* y2d) {
    *x2d = (int)(v.x * 1.5 + SRU_LARG/2);
    *y2d = (int)(SRU_ALT/2 - v.y);
}

// Projeção Ortogonal Lado (Plano XZ)
void projetarLado(Vertex v, int* x2d, int* y2d){
    *x2d = (int)(v.x * 1.5 + SRU_LARG/2);
    *y2d = (int)(SRU_ALT/2 - (v.z - ALTURA/2)); 
}

void construirCilindro(Vertex** vertices, int* numVertices, Face** faces, int* numFaces) {
    *numVertices = 2*N + 2;
    *vertices = (Vertex*)malloc((*numVertices) * sizeof(Vertex));

    // Vértices bases e centros
    for(int i = 0; i < N; i++) {
        float ang = 2*M_PI*i/N;
        (*vertices)[i] = (Vertex){RAIO * cos(ang), RAIO * sin(ang), 0};
        (*vertices)[i+N] = (Vertex){RAIO * cos(ang), RAIO * sin(ang), ALTURA};
    }
    (*vertices)[2*N] = (Vertex){0, 0, 0};       // Centro base inf
    (*vertices)[2*N+1] = (Vertex){0, 0, ALTURA}; // Centro base sup

    *numFaces = 4*N;
    *faces = (Face*)malloc((*numFaces) * sizeof(Face));
    int f = 0;

    for(int i = 0; i < N; i++) {
        int prox = (i+1)%N;
        // Lateral (2 triângulos por segmento)
        (*faces)[f++] = (Face){{i, prox, i+N}, {NULL, NULL, NULL}};
        (*faces)[f++] = (Face){{prox, prox+N, i+N}, {NULL, NULL, NULL}};
        // Tampas
        (*faces)[f++] = (Face){{2*N, prox, i}, {NULL, NULL, NULL}};
        (*faces)[f++] = (Face){{2*N+1, i+N, prox+N}, {NULL, NULL, NULL}};
    }

    // Lógica Opposite-Face (Vizinhança)
    for(int i = 0; i < *numFaces; i++) {
        for(int j = i+1; j < *numFaces; j++) {
            for(int e1 = 0; e1 < 3; e1++) {
                for(int e2 = 0; e2 < 3; e2++) {
                    if((*faces)[i].v[e1] == (*faces)[j].v[(e2+1)%3] && 
                       (*faces)[i].v[(e1+1)%3] == (*faces)[j].v[e2]) {
                        (*faces)[i].opposite[e1] = &((*faces)[j]);
                        (*faces)[j].opposite[e2] = &((*faces)[i]);
                    }
                }
            }
        }
    }
}

void renderizar(Vertex* vertices, Face* faces, int numFaces, int tipo) {
    for(int i = 0; i < numFaces; i++) {
        for(int e = 0; e < 3; e++) {
            // Desenha apenas arestas únicas usando a estrutura opposite
            if (faces[i].opposite[e] == NULL || faces[i].opposite[e] > &faces[i]) {
                int x1, y1, x2, y2;
                if(tipo == 0) {
                    projetarTopo(vertices[faces[i].v[e]], &x1, &y1);
                    projetarTopo(vertices[faces[i].v[(e+1)%3]], &x2, &y2);
                } else {
                    projetarLado(vertices[faces[i].v[e]], &x1, &y1);
                    projetarLado(vertices[faces[i].v[(e+1)%3]], &x2, &y2);
                }
                desenharLinha(x1, y1, x2, y2);
            }
        }
    }
}

int main() {
    Vertex* vertices; Face* faces; int numVertices, numFaces;
    construirCilindro(&vertices, &numVertices, &faces, &numFaces);

    printf("--- VISAO DE TOPO (Plano XY) ---\n");
    limparSRU();
    renderizar(vertices, faces, numFaces, 0);
    imprimirSRU();

    printf("\n--- PRESSIONE ENTER PARA VER A VISAO DE LADO ---\n");
    getchar();

    printf("--- VISAO DE LADO (Plano XZ) ---\n");
    limparSRU(); // Limpa para não sobrepor
    renderizar(vertices, faces, numFaces, 1);
    imprimirSRU();

    free(vertices); free(faces);
    return 0;
}
