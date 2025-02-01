#include "CViewSetup.hpp"

#include "SAR.hpp"

template <typename T>
void ViewSetupRead(T *cview, ViewSetup *view) {
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

template <typename T>
void ViewSetupWrite(ViewSetup *view, T *cview) {
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

static ViewSetup *viewCtx = new ViewSetup;
ViewSetup *ViewSetupCreate(CViewSetup *cview) {
#ifdef _WIN32
    if (sar.game->Is(SourceGame_INFRA | SourceGame_BeginnersGuide | SourceGame_StanleyParable)) {
        ViewSetupRead(reinterpret_cast<CViewSetupINFRA *>(cview), viewCtx);
    } else
#endif
        ViewSetupRead(cview, viewCtx);
    return viewCtx;
}

void ViewSetupCopy(ViewSetup *view, CViewSetup *cview) {
#ifdef _WIN32
    if (sar.game->Is(SourceGame_INFRA | SourceGame_BeginnersGuide | SourceGame_StanleyParable)) {
        ViewSetupWrite(view, reinterpret_cast<CViewSetupINFRA *>(cview));
    } else
#endif
        ViewSetupWrite(view, cview);
}
