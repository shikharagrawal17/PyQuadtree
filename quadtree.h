#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

typedef struct pqt_Boundry   pqt_Boundry;
typedef struct pqt_LeafData pqt_LeafData;
typedef struct pqt_Leaf     pqt_Leaf;
typedef struct pqt_Qnode    pqt_Qnode;
typedef void (*pqt_LeafCallback)(pqt_LeafData *, void *arg);

typedef struct pqt_Boundry {
  float xmin;
  float ymin;
  float xmax;
  float ymax;
}pqt_Boundry;

typedef struct pqt_LeafData {
  void *data;
  float x;
  float y;
  pqt_LeafData *next;
}pqt_LeafData;

typedef struct pqt_Leaf {
  union {
    pqt_Qnode *leaf;
    pqt_LeafData *payload;
  }contents;
  pqt_Boundry boundry;
  unsigned int size;
}pqf_Leaf;

 typedef struct pqt_Qnode {
  pqt_Leaf nw;
  pqt_Leaf ne;
  pqt_Leaf sw;
  pqt_Leaf se;
  unsigned int depth;
}pqt_Qnode;

typedef struct quadtree {
  pqt_Boundry boundry;
  unsigned int maxsize;
  unsigned int maxdepth;
  pqt_Qnode *head;
} pqt_QuadTree;

void pqt_init(pqt_QuadTree *qt, unsigned int  maxsize, unsigned int maxdepth, pqt_Boundry boundry);

int pqt_addpoint(pqt_QuadTree *qt, float x, float y, void *data);

int pqt_movepoint(pqt_QuadTree *qt, float old_x, float old_y, float new_x, float new_y, void *data);

void pqt_allpoints(pqt_QuadTree *qt);

void pqt_findneighbour(pqt_QuadTree *qt, float x, float y, float radius);

typedef void (*pqt_LeafCallback)(pqt_LeafData *, void *arg);
void pqt_maptonearby(pqt_QuadTree *qt, pqt_LeafCallback, void *arg, float x, float y, float radius);


//Find Leaf and return pointer to it
pqt_Leaf * pqt_findleaf(pqt_QuadTree *qt, float x, float y);

int pqt_deletepoint(pqt_QuadTree *qt, float x, float y, void *data);


void pqt_maptonearby(pqt_QuadTree *qt, pqt_LeafCallback, void *arg, float x, float y, float radius);

void pqt_deletetree(pqt_QuadTree *qt, pqt_LeafCallback visitor);

void pqt_deleteqnode(pqt_Qnode *node, pqt_LeafCallback visitor);

void pqt_deleteleaf(pqt_Leaf *leaf, pqt_LeafCallback visitor);
