#include "quadtree.h"

//Midvalue Utility Function
static inline float pqt_halfway(float min, float max)
{
  return min + ((max-min)/2.0);
}

//Check given point is in leaf or not
static inline bool pqt_pointinside(float x, float y, pqt_Boundry *boundry)
{
  return ( (x > boundry->xmin) && (x < boundry->xmax) &&
           (y > boundry->ymin) && (y < boundry->ymax) );
}

//Decide between nw, ne, sw, se
#define DO_QUADRENT(x, y, fn, node)          \
  if(x < xmid) {                            \
    if(y < ymid) {                          \
      fn(node->sw);                         \
    } else {                                \
      fn(node->nw);                         \
    }                                       \
  } else {                                  \
    if(y < ymid) {                          \
      fn(node->se);                         \
    } else {                                \
      fn(node->ne);                         \
    }                                       \
  }

#define MOVE_QUADRENT(to)                     \
  pqt_LeafData *next = cur->next;           \
  pqt_LeafData *tmp  = to.contents.payload; \
  to.contents.payload = cur;                \
  cur->next = tmp;                          \
  cur = next;                               \
  to.size++;

#define BUILD_BOUNDRY(xmin,ymin,xmax,ymax)  \
  (struct pqt_Boundry){xmin,ymin,xmax,ymax}

void pqt_leafpushdown(pqt_Leaf *leaf, unsigned int depth)
{
  float xmid = pqt_halfway(leaf->boundry.xmin, leaf->boundry.xmax);
  float ymid = pqt_halfway(leaf->boundry.ymin, leaf->boundry.ymax);

  pqt_Qnode *newnode = calloc(1, sizeof(pqt_Qnode));
  if(newnode) {
    pqt_LeafData *cur = leaf->contents.payload;

    newnode->depth = depth;

    newnode->sw.boundry = BUILD_BOUNDRY(leaf->boundry.xmin, leaf->boundry.ymin,
                                        xmid, ymid);

    newnode->se.boundry = BUILD_BOUNDRY(xmid, leaf->boundry.ymin,
                                        leaf->boundry.xmax, ymid);

    newnode->nw.boundry = BUILD_BOUNDRY(leaf->boundry.xmin, ymid,
                                        xmid, leaf->boundry.ymax);

    newnode->ne.boundry = BUILD_BOUNDRY(xmid, ymid,
                                        leaf->boundry.xmax, leaf->boundry.ymax);
    while(cur) {
      DO_QUADRENT(cur->x, cur->y, MOVE_QUADRENT, newnode);
    }
    leaf->size = 0;
    leaf->contents.leaf = newnode;
  }
}

void pqt_init(pqt_QuadTree *qt,
                 unsigned int maxsize,
                 unsigned int maxdepth,
                 pqt_Boundry boundry)
{
  float xmid = pqt_halfway(boundry.xmin,boundry.xmax);
  float ymid = pqt_halfway(boundry.ymin,boundry.ymax);

  qt->head     = NULL;
  qt->maxsize  = maxsize;
  qt->maxdepth = maxdepth;
  qt->boundry  = boundry;

  qt->head = calloc(1,sizeof(pqt_Qnode));

  if (!qt->head) {     // calloc has thrown a hizzy!
    return;
  }

  qt->head->depth = 1;

  qt->head->sw.boundry = BUILD_BOUNDRY(qt->boundry.xmin,
                                      qt->boundry.ymin,
                                      xmid,
                                      ymid);
  qt->head->se.boundry = BUILD_BOUNDRY(xmid,
                                      qt->boundry.ymin,
                                      qt->boundry.xmax,
                                      ymid);
  qt->head->nw.boundry = BUILD_BOUNDRY(qt->boundry.xmin,
                                      ymid,
                                      xmid,
                                      qt->boundry.ymax);
  qt->head->ne.boundry = BUILD_BOUNDRY(xmid,
                                      ymid,
                                      qt->boundry.xmax,
                                      qt->boundry.ymax);

}

pqt_LeafData *pqt_addleafdata(pqt_QuadTree *qt,
                              pqt_Qnode *curnode,
                              pqt_Leaf *leaf,
                              float x, float y,
                              pqt_LeafData *newnode)
{
  newnode->next          = leaf->contents.payload;
  leaf->contents.payload = newnode;

  leaf->size++;
  if((leaf->size >= qt->maxsize)&&(curnode->depth < qt->maxdepth)) {
    pqt_leafpushdown(leaf, curnode->depth+1);
  }
  return newnode;
}

#define FIND_QUADRENT(curquadrent)                                \
if((curquadrent.size==0)&&(curquadrent.contents.leaf)) {          \
  return(pqt_findleafx(curquadrent.contents.leaf,x,y));     \
} else {                                                      \
  return(&curquadrent);                                         \
}

pqt_Leaf *pqt_findleafx(pqt_Qnode *cur, float x, float y)
{
  float xmid = cur->nw.boundry.xmax;
  float ymid = cur->nw.boundry.ymax;

  DO_QUADRENT(x, y, FIND_QUADRENT, cur);
}

pqt_Leaf *pqt_findleaf(pqt_QuadTree *qt, float x, float y)
{
  if(!(qt->head)){
    return NULL;
  }
  return pqt_findleafx(qt->head, x, y);
}

#define ADD_QUADRENT(curquadrent)                                                 \
if((curquadrent.size==0)&&(curquadrent.contents.leaf)) {                          \
  pqt_addpointx(qt,curquadrent.contents.leaf,x,y,newnode);                      \
} else {                                                                      \
  pqt_addleafdata(qt, cur, &curquadrent, x, y, newnode);                        \
}

pqt_Qnode *pqt_addpointx(pqt_QuadTree * qt, pqt_Qnode *cur,
                 float x, float y,
                 pqt_LeafData *newnode)
{
  float xmid = cur->nw.boundry.xmax;
  float ymid = cur->nw.boundry.ymax;

  DO_QUADRENT(x, y, ADD_QUADRENT, cur);
  return cur;
}

int pqt_addpoint(pqt_QuadTree *qt,
                  float x, float y,
                  void *data)
{
  pqt_LeafData *newnode = malloc(sizeof(pqt_LeafData));

  if (!(newnode)){
    return 0;
  }

  newnode->data     = data;
  newnode->x        = x;
  newnode->y        = y;

  qt->head = pqt_addpointx(qt, qt->head,
                           x, y,
                           newnode);
  return 1;
}

int pqt_movepoint(pqt_QuadTree *qt,
                  float oldx, float oldy,
                  float newx, float newy,
                  void *data)
{
  pqt_Leaf *oldleaf = pqt_findleaf(qt, oldx, oldy);
  if (oldleaf->size == 0) {
    printf ("QuadTree -- point to move is not in Leaf\n");
    return 0;
  }

  pqt_LeafData *cur  = oldleaf->contents.payload;
  pqt_LeafData *prev = NULL;
  while(cur) {
    if((data == cur->data)&&(oldx==cur->x)&&(oldy==cur->y)) {
      cur->x = newx;
      cur->y = newy;
      if(!(pqt_pointinside(newx, newy, &oldleaf->boundry))) {
        if(prev){
          prev->next = cur->next;
        } else {
          oldleaf->contents.payload = cur->next;
        }
        oldleaf->size -= 1;
        pqt_addpointx(qt, qt->head,
                      newx, newy,
                      cur);
        return 1;
      }
    }
    prev = cur;
    cur  = cur->next;
  }
  return 0;
}

#define PRINT_QUADRENT(curnode, curquadrent)                        \
  printf("leaf (%f,%f) (%f,%f)\n",                              \
         curquadrent.boundry.xmin,                                \
         curquadrent.boundry.ymin,                                \
         curquadrent.boundry.xmax,                                \
         curquadrent.boundry.ymax);                               \
  if((curquadrent.contents.leaf)&&                                \
     (curquadrent.size==0)) {                                     \
    printf("descending\n");                                     \
    pqt_listpointsx(curquadrent.contents.leaf);                   \
  } else if(curquadrent.contents.payload) {                       \
    printf("contents:\n");                                      \
    pqt_LeafData *curdata = curquadrent.contents.payload;         \
    while(curdata){                                             \
      printf("(%4.2f,%4.2f) - %p\n",                            \
             curdata->x,                                        \
             curdata->y,                                        \
             curdata->data);                                    \
      curdata = curdata->next;                                  \
    }                                                           \
  } else {                                                      \
    printf("empty\n");                                          \
  }

void pqt_listpointsx(pqt_Qnode *cur)
{
  PRINT_QUADRENT(cur,cur->nw);
  PRINT_QUADRENT(cur,cur->ne);
  PRINT_QUADRENT(cur,cur->sw);
  PRINT_QUADRENT(cur,cur->se);
}

void pqt_listpoints(pqt_QuadTree *qt)
{
  pqt_listpointsx(qt->head);
}

bool pqt_overlap (pqt_Boundry boundry1, pqt_Boundry boundry2)
{
  return (!(boundry1.xmin > boundry2.xmax ||
            boundry2.xmin > boundry1.xmax ||
            boundry1.ymin > boundry2.ymax ||
            boundry2.ymin > boundry1.ymax));
}

void pqt_testoverlap(void)
{
  assert(!pqt_overlap((struct pqt_Boundry){10,10,20,20}, (struct pqt_Boundry){30,30,40,40}));
  assert(!pqt_overlap((struct pqt_Boundry){30,30,40,40}, (struct pqt_Boundry){10,10,20,20}));
  assert( pqt_overlap((struct pqt_Boundry){10,10,20,20}, (struct pqt_Boundry){0,0,40,40}  ));
  assert( pqt_overlap((struct pqt_Boundry){0,0,40,40},   (struct pqt_Boundry){10,10,20,20}));
}

float pqt_getdistance(float x1,float y1,float x2,float y2) {
  return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

#define FIND_IN_QUADRENT(curleaf)                                           \
  if(pqt_overlap(curleaf.boundry,                                         \
             (struct pqt_Boundry){x-radius,y-radius,                       \
                                 x+radius,y+radius})) {                   \
    if((curleaf.contents.leaf)&&(curleaf.size == 0)) {                    \
      pqt_maptonearbyx(curleaf.contents.leaf, visitor, arg,               \
                   curleaf.boundry,                                       \
                   x, y, radius);                                         \
    } else if ((curleaf.contents.payload)&&(curleaf.size > 0)) {          \
      pqt_LeafData *cur = curleaf.contents.payload;                       \
      while(cur) {                                                        \
        if(pqt_getdistance(x,y,cur->x,cur->y) <= radius) {                \
          visitor(cur, arg);                                              \
        }                                                                 \
        cur = cur->next;                                                  \
      }                                                                   \
    }                                                                     \
  }

void pqt_printlocation(pqt_LeafData *cur, void *arg)
{
  printf ("found -- (%f,%f)\n",cur->x,cur->y);
}

void pqt_maptonearbyx(pqt_Qnode *node, pqt_LeafCallback visitor, void *arg,
                      pqt_Boundry boundry,
                      float x, float y, float radius)
{
  FIND_IN_QUADRENT(node->nw);
  FIND_IN_QUADRENT(node->ne);
  FIND_IN_QUADRENT(node->sw);
  FIND_IN_QUADRENT(node->se);
}

void pqt_findneighbour(pqt_QuadTree *qt, float x, float y, float radius)
{
  pqt_maptonearbyx(qt->head, &pqt_printlocation, NULL, qt->boundry, x, y, radius);
}

void pqt_maptonearby(pqt_QuadTree *qt, pqt_LeafCallback visitor, void *arg,
                 float x, float y, float radius)
{
  pqt_maptonearbyx(qt->head, visitor, arg, qt->boundry, x, y, radius);
}

int pqt_deletepoint(pqt_QuadTree *qt,
                     float x, float y,
                     void *data)
{
  pqt_Leaf *leaf = pqt_findleaf(qt, x, y);
  if(leaf->size){
    pqt_LeafData *cur = leaf->contents.payload;
    pqt_LeafData *prev = NULL;
    while(cur){
      if(cur->data == data){
        if(prev){
          prev->next = cur->next;
        } else {
          leaf->contents.payload = cur->next;
        }
        free(cur);
        leaf->size -= 1;
        return 1;
      }
      prev = cur;
      cur  = cur->next;
    }
  }
  return 0;
}

void pqt_deleteqnode(pqt_Qnode *node, pqt_LeafCallback visitor)
{
  pqt_deleteleaf(&node->nw,visitor);
  pqt_deleteleaf(&node->ne,visitor);
  pqt_deleteleaf(&node->sw,visitor);
  pqt_deleteleaf(&node->se,visitor);
  free(node);
}

void pqt_deleteleaf(pqt_Leaf *leaf, pqt_LeafCallback visitor)
{
  if(leaf->size) {
    pqt_LeafData *cur = leaf->contents.payload;
    while(cur){
      pqt_LeafData *next = cur->next;
      if(visitor){
        visitor(cur, NULL);
      }
      free(cur);
      cur = next;
    }
  } else if(leaf->contents.leaf){
    pqt_deleteqnode(leaf->contents.leaf,visitor);
  }

}

void pqt_deletetree(pqt_QuadTree *qt, pqt_LeafCallback visitor)
{
  pqt_deleteqnode(qt->head, visitor);
  qt->head = NULL;
}

