#ifndef overlapchecks_h
#define overlapchecks_h

typedef int (*overlap_check)(const primitive*,const primitive*);

int default_overlap_check(const primitive*, const primitive*);
int box_box_overlap_check(const box *box1, const box *box2);
int box_heightfield_overlap_check(const box *pbox, const heightfield *phf);
int heightfield_box_overlap_check(const heightfield *phf, const box *pbox);
int box_tri_overlap_check(const box *pbox, const triangle *ptri);
int tri_box_overlap_check(const triangle *ptri, const box *pbox);
int box_ray_overlap_check(const box *pbox, const ray *pray);
int ray_box_overlap_check(const ray *pray, const box *pbox);
int box_sphere_overlap_check(const box *pbox, const sphere *psph);
int sphere_box_overlap_check(const sphere *psph, const box *pbox);
int tri_sphere_overlap_check(const triangle *ptri, const sphere *psph);
int sphere_tri_overlap_check(const sphere *psph, const triangle *ptri);
int heightfield_sphere_overlap_check(const heightfield *phf, const sphere *psph);
int sphere_heightfield_overlap_check(const sphere *psph, const heightfield *phf);
int sphere_sphere_overlap_check(const sphere *psph1, const sphere *psph2);

quotientf tri_sphere_dist2(const triangle *ptri, const sphere *psph, int &bFace);

class COverlapChecker {
public:
	COverlapChecker();
	void Init() { iPrevCode = -1; }
	int Check(int type1,int type2, primitive *prim1,primitive *prim2) {
		return table[type1][type2](prim1,prim2);
	}
	int CheckExists(int type1,int type2) {
		return table[type1][type2]!=default_overlap_check;
	}

	overlap_check table[NPRIMS][NPRIMS];
	int iPrevCode;
	float Basis21[9];
	float Basis21abs[9];
};
extern COverlapChecker g_Overlapper;

#endif