#include "CViewSetup.hpp"

void ViewSetupFromCViewSetup(CViewSetup *cview, ViewSetup *view) {
	view->origin = cview->origin;
	view->angles = cview->angles;
	view->fov = cview->fov;
	view->m_nMotionBlurMode = cview->m_nMotionBlurMode;
	view->m_bOrtho = cview->m_bOrtho;
	view->m_OrthoRight = cview->m_OrthoRight;
	view->m_OrthoLeft = cview->m_OrthoLeft;
	view->m_OrthoBottom = cview->m_OrthoBottom;
	view->m_OrthoTop = cview->m_OrthoTop;
	view->zNear = cview->zNear;
}

void ViewSetupFromCViewSetupV1(CViewSetupV1 *cview, ViewSetup *view) {
    view->origin = cview->origin;
	view->angles = cview->angles;
	view->fov = cview->fov;
	view->m_nMotionBlurMode = 0;
	view->m_bOrtho = cview->m_bOrtho;
	view->m_OrthoRight = cview->m_OrthoRight;
	view->m_OrthoLeft = cview->m_OrthoLeft;
	view->m_OrthoBottom = cview->m_OrthoBottom;
	view->m_OrthoTop = cview->m_OrthoTop;
	view->zNear = cview->zNear;
}

void ViewSetupToCViewSetup(ViewSetup *view, CViewSetup *cview) {
	cview->origin = view->origin;
	cview->angles = view->angles;
	cview->fov = view->fov;
	cview->m_nMotionBlurMode = view->m_nMotionBlurMode;
	cview->m_bOrtho = view->m_bOrtho;
	cview->m_OrthoRight = view->m_OrthoRight;
	cview->m_OrthoLeft = view->m_OrthoLeft;
	cview->m_OrthoBottom = view->m_OrthoBottom;
	cview->m_OrthoTop = view->m_OrthoTop;
	cview->zNear = view->zNear;
}

void ViewSetupToCViewSetupV1(ViewSetup *view, CViewSetupV1 *cview) {
	cview->origin = view->origin;
	cview->angles = view->angles;
	cview->fov = view->fov;
	cview->m_bOrtho = view->m_bOrtho;
	cview->m_OrthoRight = view->m_OrthoRight;
	cview->m_OrthoLeft = view->m_OrthoLeft;
	cview->m_OrthoBottom = view->m_OrthoBottom;
	cview->m_OrthoTop = view->m_OrthoTop;
	cview->zNear = view->zNear;
}

static ViewSetup *viewCtx = new ViewSetup;
ViewSetup *ViewSetupCreate(CViewSetup *cview) {
    if (cview->y == 0) {
        ViewSetupFromCViewSetup(cview, viewCtx);
    } else {
        ViewSetupFromCViewSetupV1(reinterpret_cast<CViewSetupV1 *>(cview), viewCtx);
    }
    return viewCtx;
}

void ViewSetupCopy(ViewSetup *view, CViewSetup *cview) {
    if (cview->y == 0) {
        ViewSetupToCViewSetup(view, cview);
    } else {
        ViewSetupToCViewSetupV1(view, reinterpret_cast<CViewSetupV1 *>(cview));
    }
}
