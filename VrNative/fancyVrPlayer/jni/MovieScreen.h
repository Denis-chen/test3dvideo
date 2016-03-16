/*
 * MovieScreen.h
 *
 *  Created on: 2016-3-16
 *      Author: Administrator
 */

#ifndef MOVIESCREEN_H_
#define MOVIESCREEN_H_

#include "GlGeometry.h"

//��Ļ�ļ�����
enum SCREEN_GEOMETRY
{
	SG_SPHERE, 	//���� ���ڲ���ȫ��ӰƬ
	SG_QUAD		//��Ƭ ���ڲ��ŷ�ȫ��ӰƬ
};

//��ͼӳ��ķ�ʽ
enum MOVIE_MAPPING
{
	MM_LEFT_RIGHT,	//����ӳ�� ӰƬ����벿�֣�����ӳ��ӰƬ���Ұ벿��
	MM_TOP_BOTTOM,	//����ӳ�� ӰƬ���ϰ벿�֣�����ӳ��ӰƬ���°벿��
	MM_ONLY_LEFT,	//�����۶�ӳ�� ӰƬ����벿��
	MM_ONLY_TOP,	//�����۶�ӳ�� ӰƬ���ϰ벿��
	MM_WHOLE,		//�����۶�����ӰƬ��ȫ��
	MM_VR_PORN		//vr pornģʽ
};

struct SphereScreenConfig
{
	SphereScreenConfig()
	{
		tc_mode=MM_LEFT_RIGHT;
	}
	MOVIE_MAPPING tc_mode;
};
struct QuadScreenConfig
{
	QuadScreenConfig()
	{
		width_scale=1.5;
		height_scale=0.75;
		distance_to_eye=-1.5;
		tc_mode=MM_LEFT_RIGHT;
	}
	float width_scale;
	float height_scale;
	float distance_to_eye;
	MOVIE_MAPPING tc_mode;
};
namespace OVR{
	class SurfaceTexture;
	class OvrSceneView;
}
class ShaderManager;

class MovieScreen
{
public :
	MovieScreen(){}
	virtual ~MovieScreen(){}
	virtual void Render(OVR::OvrSceneView* Scene,OVR::SurfaceTexture* MovieTexture,ShaderManager* pMng,int eye,float fovDegrees)=0;
};

class MoiveScreenSphere:public MovieScreen
{
public:
	MoiveScreenSphere();
	virtual ~MoiveScreenSphere();
	virtual void Render(OVR::OvrSceneView* Scene,OVR::SurfaceTexture* MovieTexture,ShaderManager* pMng,int eye,float fovDegrees);

	void SetConfig(const SphereScreenConfig& cfg);
private:
	OVR::GlGeometry	Globe;
	SphereScreenConfig m_cfg;
};

class MoiveScreenQuad:public MovieScreen
{
public:
	MoiveScreenQuad();
	virtual ~MoiveScreenQuad();
	virtual void Render(OVR::OvrSceneView* Scene,OVR::SurfaceTexture* MovieTexture,ShaderManager* pMng,int eye,float fovDegrees);
	void SetConfig(const QuadScreenConfig& cfg);
private:
	OVR::GlGeometry eye_quad;
	OVR::GlGeometry	Globe;
	QuadScreenConfig m_cfg;
};
#endif /* MOVIESCREEN_H_ */
