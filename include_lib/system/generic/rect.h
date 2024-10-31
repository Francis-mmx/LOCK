#ifndef RECT_H
#define RECT_H

#include "typedef.h"



struct position {
    int x;
    int y;
};

struct rect {
    int left;
    int top;
    int width;
    int height;
};


#define rect_right(r) 		((r)->left + (r)->width)
#define rect_bottom(r) 		((r)->top + (r)->height)

//#define rect_height(v)  ((v)->bottom - (v)->top)
//#define rect_width(v)  ((v)->right - (v)->left)


static inline int in_rect(const struct rect *rect, struct position *pos)
{
    if (rect->left <= pos->x && rect_right(rect) > pos->x) {
        if (rect->top <= pos->y && rect_bottom(rect) > pos->y) {
            return true;
        }
    }
    return false;
}


static inline bool get_rect_cover(const struct rect *a, const struct rect *b, struct rect *c)
{
    int right, bottom;

    c->top = MAX(a->top, b->top);
    c->left = MAX(a->left, b->left);
    right = MIN(rect_right(a), rect_right(b));
    bottom = MIN(rect_bottom(a), rect_bottom(b));

    if ((c->top < bottom) && (c->left < right)) {
        c->width = right - c->left;
        c->height = bottom - c->top;
        return true;
    }

    return false;
}

static inline void pos_limit_in_parent_persent(struct rect *pa)
{

    /*
     * 限制子控件坐标在父控件内
     */
    if ((pa->left + pa->width - 1) > 10000) {
        pa->left = 10000 - pa->width + 1;
    }
    if ((pa->top + pa->height - 1) > 10000) {
        pa->top = 10000 - pa->height + 1;
    }
    if (pa->left < 0) {
        pa->left = 0;
    }
    if (pa->top < 0) {
        pa->top = 0;
    }

}





#endif

