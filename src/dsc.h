//-----------------------------------------------------------------------------
// Data structures used frequently in the program, various kinds of vectors
// (of real numbers, not symbolic algebra stuff) and our templated lists.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#ifndef __DSC_H
#define __DSC_H

#include "solvespace.h"

class Vector;
class Vector4;
class Point2d;
class hEntity;
class hParam;

class Quaternion {
public:
    // a + (vx)*i + (vy)*j + (vz)*k
    double w, vx, vy, vz;

    static const Quaternion IDENTITY;

    static Quaternion From(double w, double vx, double vy, double vz);
    static Quaternion From(hParam w, hParam vx, hParam vy, hParam vz);
    static Quaternion From(Vector u, Vector v);
    static Quaternion From(Vector axis, double dtheta);

    Quaternion Plus(Quaternion b) const;
    Quaternion Minus(Quaternion b) const;
    Quaternion ScaledBy(double s) const;
    double Magnitude() const;
    Quaternion WithMagnitude(double s) const;

    // Call a rotation matrix [ u' v' n' ]'; this returns the first and
    // second rows, where that matrix is generated by this quaternion
    Vector RotationU() const;
    Vector RotationV() const;
    Vector RotationN() const;
    Vector Rotate(Vector p) const;

    Quaternion ToThe(double p) const;
    Quaternion Inverse() const;
    Quaternion Times(Quaternion b) const;
    Quaternion Mirror() const;
};

class Vector {
public:
    double x, y, z;

    static Vector From(double x, double y, double z);
    static Vector From(hParam x, hParam y, hParam z);
    static Vector AtIntersectionOfPlanes(Vector n1, double d1,
                                         Vector n2, double d2);
    static Vector AtIntersectionOfLines(Vector a0, Vector a1,
                                        Vector b0, Vector b1,
                                        bool *skew,
                                        double *pa=NULL, double *pb=NULL);
    static Vector AtIntersectionOfPlaneAndLine(Vector n, double d,
                                               Vector p0, Vector p1,
                                               bool *parallel);
    static Vector AtIntersectionOfPlanes(Vector na, double da,
                                         Vector nb, double db,
                                         Vector nc, double dc, bool *parallel);
    static void ClosestPointBetweenLines(Vector pa, Vector da,
                                         Vector pb, Vector db,
                                         double *ta, double *tb);

    double Element(int i) const;
    bool Equals(Vector v, double tol=LENGTH_EPS) const;
    bool EqualsExactly(Vector v) const;
    Vector Plus(Vector b) const;
    Vector Minus(Vector b) const;
    Vector Negated() const;
    Vector Cross(Vector b) const;
    double DirectionCosineWith(Vector b) const;
    double Dot(Vector b) const;
    Vector Normal(int which) const;
    Vector RotatedAbout(Vector orig, Vector axis, double theta) const;
    Vector RotatedAbout(Vector axis, double theta) const;
    Vector DotInToCsys(Vector u, Vector v, Vector n) const;
    Vector ScaleOutOfCsys(Vector u, Vector v, Vector n) const;
    double DistanceToLine(Vector p0, Vector dp) const;
    bool OnLineSegment(Vector a, Vector b, double tol=LENGTH_EPS) const;
    Vector ClosestPointOnLine(Vector p0, Vector deltal) const;
    double Magnitude() const;
    double MagSquared() const;
    Vector WithMagnitude(double s) const;
    Vector ScaledBy(double s) const;
    Vector ProjectInto(hEntity wrkpl) const;
    Vector ProjectVectorInto(hEntity wrkpl) const;
    double DivPivoting(Vector delta) const;
    Vector ClosestOrtho() const;
    void MakeMaxMin(Vector *maxv, Vector *minv) const;
    Vector ClampWithin(double minv, double maxv) const;
    static bool BoundingBoxesDisjoint(Vector amax, Vector amin,
                                      Vector bmax, Vector bmin);
    static bool BoundingBoxIntersectsLine(Vector amax, Vector amin,
                                          Vector p0, Vector p1, bool asSegment);
    bool OutsideAndNotOn(Vector maxv, Vector minv) const;
    Vector InPerspective(Vector u, Vector v, Vector n,
                         Vector origin, double cameraTan) const;
    Point2d Project2d(Vector u, Vector v) const;
    Point2d ProjectXy() const;
    Vector4 Project4d() const;
};

class Vector4 {
public:
    double w, x, y, z;

    static Vector4 From(double w, double x, double y, double z);
    static Vector4 From(double w, Vector v3);
    static Vector4 Blend(Vector4 a, Vector4 b, double t);

    Vector4 Plus(Vector4 b) const;
    Vector4 Minus(Vector4 b) const;
    Vector4 ScaledBy(double s) const;
    Vector PerspectiveProject() const;
};

class Point2d {
public:
    double x, y;

    static Point2d From(double x, double y);
    static Point2d FromPolar(double r, double a);

    Point2d Plus(const Point2d &b) const;
    Point2d Minus(const Point2d &b) const;
    Point2d ScaledBy(double s) const;
    double DivPivoting(Point2d delta) const;
    double Dot(Point2d p) const;
    double DistanceTo(const Point2d &p) const;
    double DistanceToLine(const Point2d &p0, const Point2d &dp, bool asSegment) const;
    double Angle() const;
    double AngleTo(const Point2d &p) const;
    double Magnitude() const;
    double MagSquared() const;
    Point2d WithMagnitude(double v) const;
    Point2d Normal() const;
    bool Equals(Point2d v, double tol=LENGTH_EPS) const;
};

// A simple list
template <class T>
class List {
public:
    T   *elem;
    int  n;
    int  elemsAllocated;

    void AllocForOneMore() {
        if(n >= elemsAllocated) {
            elemsAllocated = (elemsAllocated + 32)*2;
            T *newElem = (T *)MemAlloc((size_t)elemsAllocated*sizeof(elem[0]));
            for(int i = 0; i < n; i++) {
                new(&newElem[i]) T(std::move(elem[i]));
                elem[i].~T();
            }
            MemFree(elem);
            elem = newElem;
        }
    }

    void Add(const T *t) {
        AllocForOneMore();
        new(&elem[n++]) T(*t);
    }

    void AddToBeginning(const T *t) {
        AllocForOneMore();
        new(&elem[n]) T();
        std::move_backward(elem, elem + 1, elem + n + 1);
        elem[0] = *t;
        n++;
    }

    T *First() {
        return (n == 0) ? NULL : &(elem[0]);
    }
    const T *First() const {
        return (n == 0) ? NULL : &(elem[0]);
    }
    T *NextAfter(T *prev) {
        if(!prev) return NULL;
        if(prev - elem == (n - 1)) return NULL;
        return prev + 1;
    }
    const T *NextAfter(const T *prev) const {
        if(!prev) return NULL;
        if(prev - elem == (n - 1)) return NULL;
        return prev + 1;
    }

    T *begin() { return &elem[0]; }
    T *end() { return &elem[n]; }
    const T *begin() const { return &elem[0]; }
    const T *end() const { return &elem[n]; }

    void ClearTags() {
        int i;
        for(i = 0; i < n; i++) {
            elem[i].tag = 0;
        }
    }

    void Clear() {
        for(int i = 0; i < n; i++)
            elem[i].~T();
        if(elem) MemFree(elem);
        elem = NULL;
        n = elemsAllocated = 0;
    }

    void RemoveTagged() {
        int src, dest;
        dest = 0;
        for(src = 0; src < n; src++) {
            if(elem[src].tag) {
                // this item should be deleted
            } else {
                if(src != dest) {
                    elem[dest] = elem[src];
                }
                dest++;
            }
        }
        for(int i = dest; i < n; i++)
            elem[i].~T();
        n = dest;
        // and elemsAllocated is untouched, because we didn't resize
    }

    void RemoveLast(int cnt) {
        ssassert(n >= cnt, "Removing more elements than the list contains");
        for(int i = n - cnt; i < n; i++)
            elem[i].~T();
        n -= cnt;
        // and elemsAllocated is untouched, same as in RemoveTagged
    }

    void Reverse() {
        int i;
        for(i = 0; i < (n/2); i++) {
            swap(elem[i], elem[(n-1)-i]);
        }
    }
};

// A list, where each element has an integer identifier. The list is kept
// sorted by that identifier, and items can be looked up in log n time by
// id.
template <class T, class H>
class IdList {
public:
    T     *elem;
    int   n;
    int   elemsAllocated;

    uint32_t MaximumId() {
        uint32_t id = 0;

        int i;
        for(i = 0; i < n; i++) {
            id = max(id, elem[i].h.v);
        }
        return id;
    }

    H AddAndAssignId(T *t) {
        t->h.v = (MaximumId() + 1);
        Add(t);

        return t->h;
    }

    void Add(T *t) {
        if(n >= elemsAllocated) {
            elemsAllocated = (elemsAllocated + 32)*2;
            T *newElem = (T *)MemAlloc((size_t)elemsAllocated*sizeof(elem[0]));
            for(int i = 0; i < n; i++) {
                new(&newElem[i]) T(std::move(elem[i]));
                elem[i].~T();
            }
            MemFree(elem);
            elem = newElem;
        }

        int first = 0, last = n;
        // We know that we must insert within the closed interval [first,last]
        while(first != last) {
            int mid = (first + last)/2;
            H hm = elem[mid].h;
            ssassert(hm.v != t->h.v, "Handle isn't unique");
            if(hm.v > t->h.v) {
                last = mid;
            } else if(hm.v < t->h.v) {
                first = mid + 1;
            }
        }

        int i = first;
        new(&elem[n]) T();
        std::move_backward(elem + i, elem + n, elem + n + 1);
        elem[i] = *t;
        n++;
    }

    T *FindById(H h) {
        T *t = FindByIdNoOops(h);
        ssassert(t != NULL, "Cannot find handle");
        return t;
    }

    int IndexOf(H h) {
        int first = 0, last = n-1;
        while(first <= last) {
            int mid = (first + last)/2;
            H hm = elem[mid].h;
            if(hm.v > h.v) {
                last = mid-1; // and first stays the same
            } else if(hm.v < h.v) {
                first = mid+1; // and last stays the same
            } else {
                return mid;
            }
        }
        return -1;
    }

    T *FindByIdNoOops(H h) {
        int first = 0, last = n-1;
        while(first <= last) {
            int mid = (first + last)/2;
            H hm = elem[mid].h;
            if(hm.v > h.v) {
                last = mid-1; // and first stays the same
            } else if(hm.v < h.v) {
                first = mid+1; // and last stays the same
            } else {
                return &(elem[mid]);
            }
        }
        return NULL;
    }

    T *First() {
        return (n == 0) ? NULL : &(elem[0]);
    }
    T *NextAfter(T *prev) {
        if(!prev) return NULL;
        if(prev - elem == (n - 1)) return NULL;
        return prev + 1;
    }

    T *begin() { return &elem[0]; }
    T *end() { return &elem[n]; }
    const T *begin() const { return &elem[0]; }
    const T *end() const { return &elem[n]; }

    void ClearTags() {
        int i;
        for(i = 0; i < n; i++) {
            elem[i].tag = 0;
        }
    }

    void Tag(H h, int tag) {
        int i;
        for(i = 0; i < n; i++) {
            if(elem[i].h.v == h.v) {
                elem[i].tag = tag;
            }
        }
    }

    void RemoveTagged() {
        int src, dest;
        dest = 0;
        for(src = 0; src < n; src++) {
            if(elem[src].tag) {
                // this item should be deleted
            } else {
                if(src != dest) {
                    elem[dest] = elem[src];
                }
                dest++;
            }
        }
        for(int i = dest; i < n; i++)
            elem[i].~T();
        n = dest;
        // and elemsAllocated is untouched, because we didn't resize
    }
    void RemoveById(H h) {
        ClearTags();
        FindById(h)->tag = 1;
        RemoveTagged();
    }

    void MoveSelfInto(IdList<T,H> *l) {
        l->Clear();
        *l = *this;
        elemsAllocated = n = 0;
        elem = NULL;
    }

    void DeepCopyInto(IdList<T,H> *l) {
        l->Clear();
        l->elem = (T *)MemAlloc(elemsAllocated * sizeof(elem[0]));
        for(int i = 0; i < n; i++)
            new(&l->elem[i]) T(elem[i]);
        l->elemsAllocated = elemsAllocated;
        l->n = n;
    }

    void Clear() {
        for(int i = 0; i < n; i++) {
            elem[i].Clear();
            elem[i].~T();
        }
        elemsAllocated = n = 0;
        if(elem) MemFree(elem);
        elem = NULL;
    }

};

class BandedMatrix {
public:
    enum {
        MAX_UNKNOWNS   = 16,
        RIGHT_OF_DIAG  = 1,
        LEFT_OF_DIAG   = 2
    };

    double A[MAX_UNKNOWNS][MAX_UNKNOWNS];
    double B[MAX_UNKNOWNS];
    double X[MAX_UNKNOWNS];
    int n;

    void Solve();
};

#define RGBi(r, g, b) RgbaColor::From((r), (g), (b))
#define RGBf(r, g, b) RgbaColor::FromFloat((float)(r), (float)(g), (float)(b))

// Note: sizeof(class RgbaColor) should be exactly 4
//
class RgbaColor {
public:
    uint8_t red, green, blue, alpha;

    float redF()   const { return (float)red   / 255.0f; }
    float greenF() const { return (float)green / 255.0f; }
    float blueF()  const { return (float)blue  / 255.0f; }
    float alphaF() const { return (float)alpha / 255.0f; }

    bool Equals(RgbaColor c) const {
        return
            c.red   == red   &&
            c.green == green &&
            c.blue  == blue  &&
            c.alpha == alpha;
    }

    uint32_t ToPackedIntBGRA() const {
        return
            blue |
            (uint32_t)(green << 8) |
            (uint32_t)(red << 16) |
            (uint32_t)((255 - alpha) << 24);
    }

    uint32_t ToPackedInt() const {
        return
            red |
            (uint32_t)(green << 8) |
            (uint32_t)(blue << 16) |
            (uint32_t)((255 - alpha) << 24);
    }

    uint32_t ToARGB32() const {
        return
            blue |
            (uint32_t)(green << 8) |
            (uint32_t)(red << 16) |
            (uint32_t)(alpha << 24);
    }

    static RgbaColor From(int r, int g, int b, int a = 255) {
        RgbaColor c;
        c.red   = (uint8_t)r;
        c.green = (uint8_t)g;
        c.blue  = (uint8_t)b;
        c.alpha = (uint8_t)a;
        return c;
    }

    static RgbaColor FromFloat(float r, float g, float b, float a = 1.0) {
        return From(
            (int)(255.1f * r),
            (int)(255.1f * g),
            (int)(255.1f * b),
            (int)(255.1f * a));
    }

    static RgbaColor FromPackedInt(uint32_t rgba) {
        return From(
            (int)((rgba)       & 0xff),
            (int)((rgba >> 8)  & 0xff),
            (int)((rgba >> 16) & 0xff),
            (int)(255 - ((rgba >> 24) & 0xff)));
    }

    static RgbaColor FromPackedIntBGRA(uint32_t bgra) {
        return From(
            (int)((bgra >> 16) & 0xff),
            (int)((bgra >> 8)  & 0xff),
            (int)((bgra)       & 0xff),
            (int)(255 - ((bgra >> 24) & 0xff)));
    }

};

class BBox {
public:
    Vector minp;
    Vector maxp;

    static BBox From(const Vector &p0, const Vector &p1);

    Vector GetOrigin() const;
    Vector GetExtents() const;

    void Include(const Vector &v, double r = 0.0);
    bool Overlaps(const BBox &b1) const;
    bool Contains(const Point2d &p, double r = 0.0) const;
};

#endif
