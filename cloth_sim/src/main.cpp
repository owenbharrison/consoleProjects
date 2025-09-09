#include "olcConsoleGameEngine.h"

#include "cmn_generic2.h"
using cmn::float2;

#include "cmn_utils.h"

struct Particle {
	float2 pos, vel, acc;
	bool locked=false;

	Particle() {}

	Particle(const float2& p) {
		pos=p;
	}
	
	void accelerate(const float2& a) {
		acc+=a;
	}

	void update(float dt) {
		if(locked) return;

		vel+=acc*dt;
		pos+=vel*dt;

		acc*=0;
	}

	float getStress() const {
		return vel.mag()/25;
	}
};

struct Spring {
	Particle* a=nullptr, * b=nullptr;
	float rest_len=0, stiff=0, damp=0;

	Spring() {}

	Spring(Particle& a_, Particle& b_, float s, float d) {
		a=&a_, b=&b_;
		rest_len=(a->pos-b->pos).mag();
		stiff=s;
		damp=d;
	}

	void update() {
		//hookes law spring force
		float2 sub=b->pos-a->pos;
		float2 dir=sub.norm();
		float fs=stiff*(sub.mag()-rest_len);

		//thanks gonkee
		float fd=damp*dir.dot(b->vel-a->vel);
		float2 force=(fs+fd)*dir;

		a->accelerate(force);
		b->accelerate(-force);
	}
};

class ClothSimUI : public olcConsoleGameEngine {
	float2 gravity;
	int cloth_w=0, cloth_h=0;
	int ptc_num=0;
	Particle* particles=nullptr;
	int spr_num=0;
	Spring* springs=nullptr;
	
	//stress colors
	const int stress_gradient[8]{
		FG_DARK_BLUE,
		FG_BLUE,
		FG_DARK_CYAN,
		FG_CYAN,
		FG_GREEN,
		FG_DARK_YELLOW,
		FG_RED,
		FG_DARK_RED
	};

	Particle* held_ptc=nullptr;

	float total_dt=0;

public:
	ClothSimUI() {
		m_sAppName=L"Cloth Sim";
	}

	//call it stress, while it is really just the speed of the ptc
	int stressToCol(float s) const {
		int csi=cmn::clamp(int(8*s), 0, 7);
		return stress_gradient[csi];
	}

	//2d -> 1d flattening
	int ix(int i, int j) const {
		return i+cloth_w*j;
	}

	void initCloth() {
		//make softbody
		float stiff=437.243f;
		float damp=4.97f;
		for(int i=0; i<cloth_w; i++) {
			for(int j=0; j<cloth_h; j++) {
				//map to screen coords, with a little bit of random
				float x=cmn::map(i, 0, cloth_w-1, ScreenWidth()/8, ScreenWidth()*7/8)+cmn::random(-1, 1)/3;
				float y=cmn::map(j, 0, cloth_h-1, 0, ScreenHeight()*3/5)+cmn::random(-1, 1)/3;
				Particle p({x, y});
				//if at top corners, lock in place
				p.locked=(j==0&&(i==0||i==cloth_w-1));
				particles[ix(i, j)]=p;
			}
		}

		//connect axis aligned sprs
		int k=0;
		for(int i=0; i<cloth_w; i++) {
			for(int j=0; j<cloth_h; j++) {
				//connect right if can
				if(i<cloth_w-1) springs[k++]=Spring(particles[ix(i, j)], particles[ix(i+1, j)], stiff, damp);
				//connect down if can
				if(j<cloth_h-1) springs[k++]=Spring(particles[ix(i, j)], particles[ix(i, j+1)], stiff, damp);
			}
		}
	}

	bool OnUserCreate() override {
		gravity={0, 25};

		cloth_w=10;
		cloth_h=12;

		ptc_num=cloth_w*cloth_h;
		particles=new Particle[ptc_num];

		spr_num=2*cloth_w*cloth_h-cloth_w-cloth_h;
		springs=new Spring[spr_num];

		initCloth();
		
		return true;
	}

	bool OnUserUpdate(float dt) override {
		const float2 mouse_pos(GetMouseX(), GetMouseY());

		//option for reseting
		if(m_keyNewState['R']) initCloth();

		//to set and reset held ptc
		if(m_keys[VK_SPACE].bPressed) {
			for(int i=0; i<ptc_num; i++) {
				auto& p=particles[i];
				if((p.pos-mouse_pos).mag()<5) held_ptc=&p;
			}
		}
		if(m_keys[VK_SPACE].bReleased) held_ptc=nullptr;

		//to move held ptc
		if(held_ptc) held_ptc->pos=mouse_pos;

		//update springs
		for(int i=0; i<spr_num; i++) {
			springs[i].update();
		}

		//particle kinematics
		for(int i=0; i<ptc_num; i++) {
			particles[i].accelerate(gravity);
			particles[i].update(dt);
		}

		gravity.x=3*std::sin(total_dt);
		
		total_dt+=dt;

		//background
		Fill(0, 0, ScreenWidth(), ScreenHeight(), ' ');
		
		//draw squares
		for(int i=0; i<cloth_w-1; i++) {
			for(int j=0; j<cloth_h-1; j++) {
				//get 4 corners
				const auto& a=particles[ix(i, j)];
				const auto& b=particles[ix(i+1, j)];
				const auto& c=particles[ix(i, j+1)];
				const auto& d=particles[ix(i+1, j+1)];

				//color is avg between pts of square
				float avg_stress=(a.getStress()+b.getStress()+c.getStress()+d.getStress())/4;
				auto col=stressToCol(avg_stress);
				
				//draw square
				FillTriangle(
					a.pos.x, a.pos.y,
					b.pos.x, b.pos.y,
					c.pos.x, c.pos.y,
					'*', col
				);
				FillTriangle(
					b.pos.x, b.pos.y,
					d.pos.x, d.pos.y,
					c.pos.x, c.pos.y,
					'*', col
				);
			}
		}

		//draw springs
		for(int i=0; i<spr_num; i++) {
			const auto& s=springs[i];

			//color is avg between pts of spr
			float avg_stress=(s.a->getStress()+s.b->getStress())/2;
			DrawLine(
				s.a->pos.x, s.a->pos.y,
				s.b->pos.x, s.b->pos.y,
				'#', stressToCol(avg_stress)
			);

			//draw particles
			for(int i=0; i<ptc_num; i++) {
				const auto& p=particles[i];
				Draw(p.pos.x, p.pos.y, 0x2588, stressToCol(p.getStress()));
			}
		}

		return true;
	}
};

int main() {
	ClothSimUI csui;
	if(csui.ConstructConsole(80, 100, 6, 6)) csui.Start();

	return false;
}