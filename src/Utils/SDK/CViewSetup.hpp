#pragma once

#include "Math.hpp"

struct CViewSetup {
	int x;
	int m_nUnscaledX;
	int y;
	int m_nUnscaledY;
	int width;
	int m_nUnscaledWidth;
	int height;
	int m_nUnscaledHeight;
	bool m_bOrtho;
	float m_OrthoLeft;
	float m_OrthoTop;
	float m_OrthoRight;
	float m_OrthoBottom;
	bool m_bCustomViewMatrix;
	float m_matCustomViewMatrix[3][4];
	float fov;
	float fovViewmodel;
	Vector origin;
	QAngle angles;
	float zNear;
	float zFar;
	float zNearViewmodel;
	float zFarViewmodel;
	float m_flAspectRatio;
	float m_flNearBlurDepth;
	float m_flNearFocusDepth;
	float m_flFarFocusDepth;
	float m_flFarBlurDepth;
	float m_flNearBlurRadius;
	float m_flFarBlurRadius;
	int m_nDoFQuality;
	int m_nMotionBlurMode;
	float m_flShutterTime;
	Vector m_vShutterOpenPosition;
	QAngle m_shutterOpenAngles;
	Vector m_vShutterClosePosition;
	QAngle m_shutterCloseAngles;
	float m_flOffCenterTop;
	float m_flOffCenterBottom;
	float m_flOffCenterLeft;
	float m_flOffCenterRight;
	bool m_bOffCenter : 1;
	bool m_bRenderToSubrectOfLargerScreen : 1;
	bool m_bDoBloomAndToneMapping : 1;
	bool m_bDoDepthOfField : 1;
	bool m_bHDRTarget : 1;
	bool m_bDrawWorldNormal : 1;
	bool m_bCullFrontFaces : 1;
	bool m_bCacheFullSceneState : 1;
};

struct CViewSetupV1 {
	int x;
	int y;
	int width;
	int height;
	bool clearColor;
	bool clearDepth;
	bool bForceClearWholeRenderTarget;
	bool m_bOrtho;
	float m_OrthoLeft;
	float m_OrthoTop;
	float m_OrthoRight;
	float m_OrthoBottom;
	float fov;
	float fovViewmodel;
	Vector origin;
	Vector m_vUnreflectedOrigin;
	QAngle angles;
	float zNear;
	float zFar;
	float zNearViewmodel;
	float zFarViewmodel;
	bool m_bForceAspectRatio1To1;
	bool m_bRenderToSubrectOfLargerScreen;
	bool m_bUseRenderTargetAspectRatio;
};

// Only the things we use for now
struct ViewSetup {
	Vector origin;
	QAngle angles;
	float fov;
	int m_nMotionBlurMode;
	bool m_bOrtho;
	float m_OrthoRight, m_OrthoLeft, m_OrthoBottom, m_OrthoTop;
	float zNear;
};

void ViewSetupFromCViewSetup(CViewSetup *cview, ViewSetup *view);
void ViewSetupFromCViewSetupV1(CViewSetupV1 *cview, ViewSetup *view);
void ViewSetupToCViewSetup(ViewSetup *view, CViewSetup *cview);
void ViewSetupToCViewSetupV1(ViewSetup *view, CViewSetupV1 *cview);
ViewSetup *ViewSetupCreate(CViewSetup *cview);
void ViewSetupCopy(ViewSetup *view, CViewSetup *cview);
