#include "polygon.h"
#include <QtMath>

enum {
    ENTRY = true,
    EXIT = false
};

struct vertex {
    double x, y;
    vertex *next = nullptr, *prev = nullptr;
    vertex *nextPoly = nullptr;
    vertex *neighbour = nullptr;
    double alpha = 0;
    bool intersect = false;
    bool entry_exit;
    bool processed = false;
};

bool intersect(vertex *P1, vertex *P2, vertex *Q1, vertex *Q2, double &alphaP, double &alphaQ) {
    double P1P2x = P2->x - P1->x, P1P2y = P2->y - P1->y;
    double Q1Q2x = Q2->x - Q1->x, Q1Q2y = Q2->y - Q1->y;
    double WEC_P1 = (P1->x - Q1->x) * Q1Q2y - (P1->y - Q1->y) * Q1Q2x;
    double WEC_P2 = (P2->x - Q1->x) * Q1Q2y - (P2->y - Q1->y) * Q1Q2x;
    if (WEC_P1 * WEC_P2 <= 0) {
        double WEC_Q1 = (Q1->x - P1->x) * P1P2y - (Q1->y - P1->y) * P1P2x;
        double WEC_Q2 = (Q2->x - P1->x) * P1P2y - (Q2->y - P1->y) * P1P2x;
        if (WEC_Q1 * WEC_Q2 <= 0) {
            alphaP = WEC_P1 / (WEC_P1 - WEC_P2);
            alphaQ = WEC_Q1 / (WEC_Q1 - WEC_Q2);

            // perturbation for degeneracy
            if (qAbs(alphaP) < 1e-5) {
                P1->x += 2 * Q1Q2y / qSqrt(Q1Q2x * Q1Q2x + Q1Q2y * Q1Q2y);
                P1->y -= 2 * Q1Q2x / qSqrt(Q1Q2x * Q1Q2x + Q1Q2y * Q1Q2y);
                return intersect(P1, P2, Q1, Q2, alphaP, alphaQ);
            }
            if (qAbs(alphaQ) < 1e-5) {
                Q1->x += 2 * P1P2y / qSqrt(P1P2x * P1P2x + P1P2y * P1P2y);
                Q1->y -= 2 * P1P2x / qSqrt(P1P2x * P1P2x + P1P2y * P1P2y);
                return intersect(P1, P2, Q1, Q2, alphaP, alphaQ);
            }

            return true;
        }
    }
    return false;
}

void createPolygon(vertex *&p, Polygon poly) {

    vertex *head = nullptr;

    if (poly.outerRing.vertices.isEmpty()) {
        p = nullptr;
        return;
    }

    auto createSimplePolygon = [](SimplePolygon sp) {
        vertex *head = nullptr, *tail = nullptr, *prevTail = nullptr;
        int n = sp.vertices.size();
        for (int i = 0; i < n; i++) {
            if (head == nullptr) {
                head = new vertex;
                head->x = sp.vertices[i].x;
                head->y = sp.vertices[i].y;
                tail = head;
            }
            else {
                tail->next = new vertex;
                prevTail = tail;
                tail = tail->next;
                tail->x = sp.vertices[i].x;
                tail->y = sp.vertices[i].y;
                tail->prev = prevTail;
            }
        }
        tail->next = head;
        head->prev = tail;
        return head;
    };


    head = createSimplePolygon(poly.outerRing);

    QList<vertex*> inners;
    for (int i=0; i<poly.innerRings.size(); i++) {
        inners.append(createSimplePolygon(poly.innerRings[i]));
    }
    if (!inners.isEmpty()) {
        for (int i=0; i<inners.size() - 1; i++) {
            inners[i]->nextPoly = inners[i + 1];
        }
        head->nextPoly = inners[0];
    }

    p = head;
}

vertex* createVertex(vertex *v1, vertex* v2, double alpha) {
    vertex *v = new vertex;
    v1->next = v;
    v->prev = v1;
    v->next = v2;
    v2->prev = v;

    v->alpha = alpha;
    v->intersect = true;

    return v;
}

vertex getCoordinate(vertex *v) {
    vertex result;
    if (v->intersect) {
        vertex *l = v, *r = v;
        while (l->intersect) {
            l = l->prev;
        }
        while (r->intersect) {
            r = r->next;
        }
        result.x = (1 - v->alpha) * l->x + (v->alpha) * r->x;
        result.y = (1 - v->alpha) * l->y + (v->alpha) * r->y;
    }
    else {
        result.x = v->x;
        result.y = v->y;
    }
    return result;
}

void sortIntersection(vertex *i) {
    while (i->next->intersect && i->next->alpha < i->alpha) {
        // swap i and i->next
        vertex *prev = i->prev;
        vertex *next = i->next;
        vertex *nn = next->next;

        prev->next = next;
        next->prev = prev;
        next->next = i;
        i->prev = next;
        i->next = nn;
        nn->prev = i;
    }
}

void releasePoly(vertex* poly) {
    if (poly == nullptr)
        return;

    vertex *p = poly->next, *q;
    vertex *curHead = poly;
    while (p != nullptr) {
        while (p != curHead) {
            q = p->next;
            delete p;
            p = q;
        }
        curHead = p->nextPoly;
        delete p;
        if (curHead == nullptr)
            break;

        p = curHead->next;
        q = curHead;
    }
};

QList<Polygon> Polygon::clip(Polygon subjectP, Polygon clipP) {
    // Using Greiner Hormann algorithm
    Polygon afterSub = subjectP.afterTransformation();
    Polygon afterClip = clipP.afterTransformation();

    vertex *subject = nullptr, *clip = nullptr;

    createPolygon(subject, afterSub);
    createPolygon(clip, afterClip);

    // Phase 1
    // Find intersections and insert them into the linked list
    vertex *s = subject, *c = clip;
    bool sPolyEnd = false, cPolyEnd = false;
    vertex *sCurPolyHead = s, *cCurPolyHead = c;
    while (!sPolyEnd) {
        // s is the first vertex
        if (!s->intersect) {
            // s2 is the next vertex
            vertex *s2 = s->next;
            while (s2->intersect) {
                s2 = s2->next;
            }

            while (!cPolyEnd) {
                double a, b;
                if (!c->intersect) {
                    // c2 is the next vertex
                    vertex *c2 = c->next;
                    while (c2->intersect) {
                        c2 = c2->next;
                    }

                    if (intersect(s, s2, c, c2, a, b)) {
                        vertex *i1 = createVertex(s, s->next, a);
                        vertex *i2 = createVertex(c, c->next, b);
                        i1->neighbour = i2;
                        i2->neighbour = i1;

                        sortIntersection(i1);
                        sortIntersection(i2);

                    }
                }

                // Find next vertex
                while(c->next->intersect) {
                    c = c->next;
                }
                c = c->next;
                if (c == cCurPolyHead) {
                    c = c->nextPoly;
                    cCurPolyHead = c;
                }
                if (c == nullptr)
                    cPolyEnd = true;
            }

            // Find next vertex
            while(s->next->intersect) {
                s = s->next;
            }
            s = s->next;

            if (s == sCurPolyHead) {
                s = s->nextPoly;
                sCurPolyHead = s;
            }

            // Restore c
            c = clip;
            cCurPolyHead = c;
            cPolyEnd = false;

            if (s == nullptr)
                sPolyEnd = true;
        }
    }

    // Phase 2
    // Mark entry and exit of intersections
    bool status;
    s = subject;
    sCurPolyHead = s;
    while (s != nullptr) {
        if (afterClip.isInsidePolygon(Point(static_cast<int>(s->x), static_cast<int>(s->y)))) {
            status = EXIT;
        }
        else {
            status = ENTRY;
        }

        do {
            if (s->intersect) {
                s->entry_exit = status;
                status = !status;
            }
            s = s->next;
        } while (s != sCurPolyHead);

        s = s->nextPoly;
        sCurPolyHead = s;
    }

    c = clip;
    cCurPolyHead = c;
    while (c != nullptr) {
        if (afterSub.isInsidePolygon(Point(static_cast<int>(c->x), static_cast<int>(c->y)))) {
            status = EXIT;
        }
        else {
            status = ENTRY;
        }

        do {
            if (c->intersect) {
                c->entry_exit = status;
                status = !status;
            }
            c = c->next;
        } while (c != cCurPolyHead);

        c = c->nextPoly;
        cCurPolyHead = c;
    }

    // Phase 3
    // Draw the intersect polygon
    s = subject;
    sCurPolyHead = s;
    QList<SimplePolygon> rawResult;
    while (s != nullptr) {
        do {
            s = s->next;
            if (s->intersect && !s->processed)
                break;
        } while (s != sCurPolyHead);

        if (s == sCurPolyHead) {
            s = s->nextPoly;
            sCurPolyHead = s;
            continue;
        }

        vertex *cur = s;
        SimplePolygon sp;
        vertex cor = getCoordinate(cur);
        sp.vertices.push_back(Point(static_cast<int>(cor.x), static_cast<int>(cor.y)));
        cur->processed = true;
        if (cur->neighbour)
            cur->neighbour->processed = true;

        do {
            if (cur->entry_exit == ENTRY) {
                do {
                    cur = cur->next;
                    vertex cor = getCoordinate(cur);
                    sp.vertices.push_back(Point(static_cast<int>(cor.x), static_cast<int>(cor.y)));
                    cur->processed = true;
                    if (cur->neighbour)
                        cur->neighbour->processed = true;
                } while (!cur->intersect);
            }
            else {
                do {
                    cur = cur->prev;
                    vertex cor = getCoordinate(cur);
                    sp.vertices.push_back(Point(static_cast<int>(cor.x), static_cast<int>(cor.y)));
                    cur->processed = true;
                    if (cur->neighbour)
                        cur->neighbour->processed = true;
                } while (!cur->intersect);
            }
            cur = cur->neighbour;
        } while (cur != s);

        rawResult.push_back(sp);

        // Restore s;
        s = sCurPolyHead;
    }

    // Remove duplicate vertex
    for (int i=0; i<rawResult.size();i++) {
        rawResult[i].vertices.takeAt(0);
    }

    // Phase 4
    // Find those polygons with no intersection
    s = subject;
    sCurPolyHead = s;
    while (s != nullptr) {
        SimplePolygon sp;
        bool flagNoIntersection = true;
        do {
            if (s->intersect) {
                flagNoIntersection = false;
                break;
            }
            s = s->next;
            vertex cor = getCoordinate(s);
            sp.vertices.push_back(Point(static_cast<int>(cor.x), static_cast<int>(cor.y)));
        } while (s != sCurPolyHead);

        if (flagNoIntersection && afterClip.isInsidePolygon(sp.vertices[0]))
            rawResult.push_back(sp);

        s = sCurPolyHead->nextPoly;
        sCurPolyHead = s;
    }

    c = clip;
    cCurPolyHead = c;
    while (c != nullptr) {
        SimplePolygon sp;
        bool flagNoIntersection = true;
        do {
            if (c->intersect) {
                flagNoIntersection = false;
                break;
            }
            c = c->next;
            vertex cor = getCoordinate(c);
            sp.vertices.push_back(Point(static_cast<int>(cor.x), static_cast<int>(cor.y)));
        } while (c != cCurPolyHead);

        if (flagNoIntersection && afterSub.isInsidePolygon(sp.vertices[0]))
            rawResult.push_back(sp);

        c = cCurPolyHead->nextPoly;
        cCurPolyHead = c;
    }

    // Delete redundant vertices
    for (int i = 0; i < rawResult.size(); i++) {
        int n = rawResult[i].vertices.size();
        for (int j = 0; j < n; j++) {
            Point v1 = rawResult[i].vertices[j];
            Point v2 = rawResult[i].vertices[(j + 1) % n];
            if (qAbs(v1.x - v2.x) + qAbs(v1.y - v2.y) <= 2.1) {
                rawResult[i].vertices.takeAt(j);
                j--;
                n--;
            }
        }
    }

    // Assemble raw results into polygon
    struct flagPolygon {
        bool isParent = true;
        QList<int> childs;
    };
    int rawResultSize = rawResult.size();
    QVector<flagPolygon> flagRaw(rawResultSize);
    for (int i = 0; i < rawResultSize; i++) {
        for (int j = i + 1; j < rawResultSize; j++) {
            Polygon r1(rawResult[i]), r2(rawResult[j]);
            if (r1.isInsidePolygon(rawResult[j].vertices[0])) {
                flagRaw[i].childs.append(j);
                flagRaw[j].isParent = false;
            }
            if (r2.isInsidePolygon(rawResult[i].vertices[0])) {
                flagRaw[j].childs.append(i);
                flagRaw[i].isParent = false;
            }
        }
    }

    QList<Polygon> result;
    for (int i = 0; i < rawResultSize; i++) {
        if (flagRaw[i].isParent) {
            SimplePolygon outer = rawResult[i];
            QList<SimplePolygon> inner;
            for (int j = 0; j < flagRaw[i].childs.size(); j++) {
                inner.append(rawResult[flagRaw[i].childs[j]]);
            }
            Polygon p(outer, inner);
            result.append(p);
        }
    }

    releasePoly(subject);
    releasePoly(clip);

    return result;
}
