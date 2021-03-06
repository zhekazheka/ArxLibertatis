/*
 * Copyright 2011-2016 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code').

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#include "gui/MenuWidgets.h"

#include <cctype>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iosfwd>
#include <limits>
#include <algorithm>
#include <sstream>

#include <boost/foreach.hpp>

#include "core/Application.h"
#include "core/ArxGame.h"
#include "core/Benchmark.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "core/Localisation.h"
#include "core/SaveGame.h"
#include "core/Version.h"

#include "gui/Hud.h"
#include "gui/LoadLevelScreen.h"
#include "gui/MainMenu.h"
#include "gui/Menu.h"
#include "gui/MenuPublic.h"
#include "gui/Text.h"
#include "gui/Interface.h"
#include "gui/Credits.h"
#include "gui/TextManager.h"
#include "gui/menu/MenuCursor.h"
#include "gui/menu/MenuFader.h"
#include "gui/widget/CycleTextWidget.h"
#include "gui/widget/PanelWidget.h"

#include "graphics/Draw.h"
#include "graphics/DrawLine.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/font/Font.h"
#include "graphics/texture/TextureStage.h"

#include "input/Input.h"

#include "scene/ChangeLevel.h"
#include "scene/GameSound.h"
#include "scene/LoadLevel.h"

#include "window/RenderWindow.h"

extern int newWidth;
extern int newHeight;
extern bool newFullscreen;

// Imported global variables and functions
extern ARX_MENU_DATA ARXmenu;

bool bNoMenu=false;

MenuCursor * pMenuCursor = NULL;

extern CWindowMenu * pWindowMenu;
MainMenu *mainMenu;

extern TextWidget * pMenuElementApply;

void ARX_QuickSave() {
	
	if(!g_canResumeGame) {
		return;
	}
	
	ARX_SOUND_MixerPause(ARX_SOUND_MixerGame);
	
	savegames.quicksave(savegame_thumbnail);
	
	ARX_SOUND_MixerResume(ARX_SOUND_MixerGame);
}

static bool ARX_LoadGame(const SaveGame & save) {
	
	benchmark::begin(benchmark::LoadLevel);
	
	LoadLevelScreen();
	progressBarSetTotal(238);
	progressBarReset();
	progressBarAdvance();
	LoadLevelScreen(save.level);
	
	DanaeClearLevel();
	
	long ret = ARX_CHANGELEVEL_Load(save.savefile);
	
	g_canResumeGame = true;

	return ret != -1;
}

bool ARX_QuickLoad() {
	bool loaded;

	SaveGameList::iterator save = savegames.quickload();
	if(save == savegames.end()) {
		// No saves found!
		return false;
	}

	ARX_SOUND_MixerPause(ARX_SOUND_MixerGame);
	loaded = ARX_LoadGame(*save);
	ARX_SOUND_MixerResume(ARX_SOUND_MixerGame);
	
	return loaded;
}

bool ARX_SlotLoad(SavegameHandle slotIndex) {
	if(slotIndex.handleData() >= (int)savegames.size()) {
		// Invalid slot!
		return false;
	}

	ARX_SOUND_MixerPause(ARX_SOUND_MixerMenu);
	return ARX_LoadGame(savegames[slotIndex.handleData()]);
}

bool MENU_NoActiveWindow() {
	
	if(!pWindowMenu || pWindowMenu->m_currentPageId == MAIN) {
		return true;
	}
	
	return false;
}

static void Check_Apply() {
	if(pMenuElementApply) {
		if(config.video.resolution.x != newWidth
		   || config.video.resolution.y != newHeight
		   || config.video.fullscreen != newFullscreen
		) {
			pMenuElementApply->SetCheckOn();
			pMenuElementApply->lColor = pMenuElementApply->lOldColor;
		} else {
			if(pMenuElementApply->lColor != Color(127,127,127)) {
				pMenuElementApply->SetCheckOff();
				pMenuElementApply->lOldColor = pMenuElementApply->lColor;
				pMenuElementApply->lColor = Color(127,127,127);
			}
		}
	}
}



bool Menu2_Render() {
	
	if(pMenuCursor == NULL) {
		pMenuCursor = new MenuCursor();
	}
	pMenuCursor->update(g_platformTime.lastFrameDuration());
	
	GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterLinear);
	
	if(AMCM_NEWQUEST == ARXmenu.currentmode || AMCM_CREDITS == ARXmenu.currentmode) {
		
		delete pWindowMenu, pWindowMenu = NULL;
		delete mainMenu, mainMenu = NULL;
		
		if(ARXmenu.currentmode == AMCM_CREDITS){
			credits::render();
			return true;
		}
		
		return false;
	}
	
	if(pTextManage) {
		pTextManage->Clear();
	}

	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);

	UseRenderState state(render2D());

	MENUSTATE eOldMenuState=NOP;
	MENUSTATE eM;

	if(!mainMenu) {
		eM = NOP;
	} else {
		eM = mainMenu->eOldMenuWindowState;
	}
	
	if(!mainMenu || mainMenu->bReInitAll) {
		
		if(mainMenu && mainMenu->bReInitAll) {
			eOldMenuState = mainMenu->eOldMenuState;
			delete pWindowMenu, pWindowMenu = NULL;
			delete mainMenu, mainMenu = NULL;
		}
		
		mainMenu = new MainMenu();
		mainMenu->eOldMenuWindowState=eM;
		
		mainMenu->init();
	}

	bool bScroll=true;
	
	MENUSTATE eMenuState = mainMenu->Update();
	
	if(eOldMenuState != NOP) {
		eMenuState=eOldMenuState;
		bScroll=false;
	}
	
	if(eMenuState == RESUME_GAME) {
		pTextManage->Clear();
		ARXmenu.currentmode = AMCM_OFF;
		mainMenu->m_selected = NULL;
		
		delete pWindowMenu, pWindowMenu = NULL;
		delete mainMenu, mainMenu = NULL;
		
		GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
		
		return true;
	} else if(eMenuState != NOP) {
		MainMenuLeftCreate(eMenuState);
	
	}
	
	mainMenu->Render();
	
	if(pWindowMenu)
	if(   (pWindowMenu->m_currentPageId != MAIN && pWindowMenu->m_currentPageId != NEW_QUEST && pWindowMenu->m_currentPageId != CREDITS)
	   || (pWindowMenu->m_currentPageId == NEW_QUEST && g_canResumeGame)
	) {
		if(!bScroll) {
			pWindowMenu->fAngle=90.f;
			pWindowMenu->m_currentPageId=mainMenu->eOldMenuWindowState;
		}

		pWindowMenu->Update(g_platformTime.lastFrameDuration());
		MENUSTATE eMS = pWindowMenu->Render();
		if(eMS != NOP) {
			mainMenu->eOldMenuWindowState=eMS;
		}
		Check_Apply();
	}

	bNoMenu=false;

	// If the menu needs to be reinitialized, then the text in the TextManager is probably using bad fonts that were deleted already
	// Skip one update in this case
	if(pTextManage && !mainMenu->bReInitAll) {
		pTextManage->Update(g_platformTime.lastFrameDuration());
		pTextManage->Render();
	}

	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);

	pMenuCursor->DrawCursor();
	
	g_thumbnailCursor.render();
	
	if(MenuFader_process(bFadeInOut)) {
		switch(iFadeAction) {
			case AMCM_CREDITS:
				ARX_MENU_Clicked_CREDITS();
				MenuFader_start(true, false, -1);
				break;
			case AMCM_NEWQUEST:
				ARX_MENU_Clicked_NEWQUEST();
				MenuFader_start(true, false, -1);
				cinematicBorder.reset();
				break;
			case AMCM_OFF:
				mainApp->quit();
				MenuFader_start(true, false, -1);
				break;
		}
	}

	GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
	
	return true;
}








CWindowMenu::CWindowMenu(const Vec2f & pos, const Vec2f & size)
{
	m_pos = RATIO_2(pos);
	m_size = RATIO_2(size);

	m_pages.clear();

	fPosXCalc = -m_size.x;
	fDist = m_size.x + m_pos.x;
	fAngle=0.f;

	m_currentPageId=NOP;


	float fCalc	= fPosXCalc + (fDist * glm::sin(glm::radians(fAngle)));

	m_pos.x = checked_range_cast<int>(fCalc);
	
	m_background = TextureContainer::LoadUI("graph/interface/menus/menu_console_background");
	m_border = TextureContainer::LoadUI("graph/interface/menus/menu_console_background_border");
}

CWindowMenu::~CWindowMenu() {

	for(std::vector<MenuPage*>::iterator it = m_pages.begin(), it_end = m_pages.end(); it < it_end; ++it)
		delete *it;
}

void CWindowMenu::add(MenuPage *page) {

	m_pages.push_back(page);
	page->m_oldPos.x = 0;
	page->m_oldPos.y = 0;
	page->m_pos = m_pos;
}

void CWindowMenu::Update(PlatformDuration _fDTime) {

	float fCalc	= fPosXCalc + (fDist * glm::sin(glm::radians(fAngle)));

	m_pos.x = checked_range_cast<int>(fCalc);
	fAngle += float(toMs(_fDTime)) * 0.08f;

	if(fAngle > 90.f)
		fAngle = 90.f;
}

MENUSTATE CWindowMenu::Render() {
	
	if(bNoMenu)
		return NOP;
	
	MENUSTATE eMS=NOP;
	
	{MenuPage * page; BOOST_FOREACH(page, m_pages) {
		if(m_currentPageId == page->eMenuState) {
			eMS = page->Update(m_pos);
			
			if(eMS != NOP)
				break;
		}
	}}
	
	// Draw backgound and border
	{
		UseRenderState state(render2D().blend(BlendZero, BlendInvSrcColor));
		EERIEDrawBitmap2(Rectf(Vec2f(m_pos.x, m_pos.y),
		                 RATIO_X(m_background->m_size.x), RATIO_Y(m_background->m_size.y)),
		                 0, m_background, Color::white);
	}
	
	EERIEDrawBitmap2(Rectf(Vec2f(m_pos.x, m_pos.y),
	                 RATIO_X(m_border->m_size.x), RATIO_Y(m_border->m_size.y)),
	                 0, m_border, Color::white);
	
	{MenuPage * page; BOOST_FOREACH(page, m_pages) {
		if(m_currentPageId == page->eMenuState) {
			page->Render();
			
			if(g_debugInfo == InfoPanelGuiDebug)
				page->drawDebug();

			if(eMS == NOP)
				eMS = page->checkShortcuts();
			break;
		}
	}}
	
	if(eMS != NOP) {
		m_currentPageId=eMS;
	}
	
	return eMS;
}

MenuPage::MenuPage(const Vec2f & pos, const Vec2f & size, MENUSTATE _eMenuState)
	: m_rowSpacing(10)
	, m_savegame(0)
	, m_selected(NULL)
	, bEdit(false)
	, bMouseAttack(false)
	, m_disableShortcuts(false)
	, m_blinkTime(PlatformDuration_ZERO)
	, m_blink(true)
{
	m_size = size;
	
	Vec2f scaledSize = RATIO_2(size);
	m_rect = Rectf(RATIO_2(pos), scaledSize.x, scaledSize.y);
	
	eMenuState=_eMenuState;
}

MenuPage::~MenuPage() { }

void MenuPage::add(Widget * widget) {
	widget->ePlace = NOCENTER;
	widget->Move(m_rect.topLeft());
	m_children.add(widget);
}

void MenuPage::addCenter(Widget * widget, bool centerX) {

	int dx;
	
	if(centerX) {
		widget->ePlace = CENTER;
		
		int iDx = widget->m_rect.width();
		dx  = ((m_rect.width() - iDx) / 2) - widget->m_rect.left;
	
		if(dx < 0) {
			dx = 0;
		}
	} else {
		dx = 0;
	}
	
	int iDy = widget->m_rect.height();
	
	{Widget * w; BOOST_FOREACH(w, m_children.m_widgets) {
		iDy += m_rowSpacing;
		iDy += w->m_rect.height();
	}}

	int iDepY = m_rect.left;

	if(iDy < m_rect.height()) {
		iDepY += ((m_rect.height() - iDy) / 2);
	}

	int dy = 0;

	if(!m_children.m_widgets.empty()) {
		dy = iDepY - m_children.m_widgets[0]->m_rect.top;
	}
	
	{Widget * w; BOOST_FOREACH(w, m_children.m_widgets) {
		iDepY += (w->m_rect.height()) + m_rowSpacing;
		
		w->Move(Vec2f(0, dy));
	}}

	widget->Move(Vec2f(dx, iDepY));

	m_children.add(widget);
}

void MenuPage::AlignElementCenter(Widget * widget) {
	
	widget->Move(Vec2f(-widget->m_rect.left, 0));
	widget->ePlace = CENTER;
	
	float iDx = widget->m_rect.width();
	float dx = (m_rect.width() - iDx) / 2 - widget->m_rect.left;
	
	widget->Move(Vec2f(std::max(dx, 0.f), 0.f));
}

void MenuPage::updateTextRect(TextWidget * widget) {
	float iDx = widget->m_rect.width();
	
	if(widget->ePlace) {
		widget->m_rect.left = m_pos.x + ((m_rect.width() - iDx) / 2.f);
		
		if(widget->m_rect.left < 0) {
			widget->m_rect.left = 0;
		}
	}
	
	widget->m_rect.right = widget->m_rect.left+iDx;
}

void MenuPage::UpdateText() {
	
	TextWidget * textWidget = (TextWidget*)m_selected;
	
	if(GInput->isAnyKeyPressed()) {
		
		if(GInput->isKeyPressed(Keyboard::Key_Enter)
		   || GInput->isKeyPressed(Keyboard::Key_NumPadEnter)
		   || GInput->isKeyPressed(Keyboard::Key_Escape)
		) {
			ARX_SOUND_PlayMenu(SND_MENU_CLICK);
			textWidget->eState = EDIT;
			
			if(textWidget->m_text.empty()) {
				std::string szMenuText;
				szMenuText = getLocalised("system_menu_editquest_newsavegame");
				
				textWidget->SetText(szMenuText);
				
				updateTextRect(textWidget);
			}
			
			m_selected = NULL;
			bEdit = false;
			return;
		}
		
		bool bKey = false;
		std::string tText;
		
		if(GInput->isKeyPressedNowPressed(Keyboard::Key_Backspace)) {
			tText = textWidget->m_text;
			
			if(!tText.empty()) {
				tText.resize(tText.size() - 1);
				bKey = true;
			}
		} else {
			int iKey = GInput->getKeyPressed();
			iKey &= 0xFFFF;
			
			if(GInput->isKeyPressedNowPressed(iKey)) {
				tText = textWidget->m_text;
				
				char tCat;
				
				bKey = GInput->getKeyAsText(iKey, tCat);
				
				if(bKey) {
					int iChar = tCat & 0x000000FF; // To prevent ascii chars between [128, 255] from causing an assertion in the functions below...
					if((isalnum(iChar) || isspace(iChar) || ispunct(iChar)) && (tCat != '\t') && (tCat != '*'))
						tText += tCat;
				}
			}
		}
		
		if(bKey) {
			textWidget->SetText(tText);
			
			if(textWidget->m_rect.width() > m_rect.width() - RATIO_X(64)) {
				if(!tText.empty()) {
					tText.resize(tText.size() - 1);
					textWidget->SetText(tText);
				}
			}
			
			updateTextRect(textWidget);
		}
	}
	
	if(m_selected->m_rect.top == m_selected->m_rect.bottom) {
		Vec2i textSize = ((TextWidget*)m_selected)->m_font->getTextSize("|");
		m_selected->m_rect.bottom += textSize.y;
	}
	
	if(m_blink) {
		//DRAW CURSOR
		TexturedVertex v[4];
		GRenderer->ResetTexture(0);
		v[0].color = v[1].color = v[2].color = v[3].color = Color::white.toRGB();
		v[0].p.z=v[1].p.z=v[2].p.z=v[3].p.z=0.f;
		v[0].rhw=v[1].rhw=v[2].rhw=v[3].rhw=1.f;
		
		v[0].p.x = m_selected->m_rect.right;
		v[0].p.y = m_selected->m_rect.top;
		v[1].p.x = v[0].p.x+2.f;
		v[1].p.y = v[0].p.y;
		v[2].p.x = v[0].p.x;
		v[2].p.y = m_selected->m_rect.bottom;
		v[3].p.x = v[1].p.x;
		v[3].p.y = v[2].p.y;
		
		EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
	}
}

TextWidget * MenuPage::GetTouch(bool keyTouched, int keyId, InputKeyId* pInputKeyId, bool _bValidateTest)
{
	int iMouseButton = keyTouched ? 0 : GInput->getMouseButtonClicked();
	
	if(pInputKeyId)
		*pInputKeyId = keyId;

	if(keyTouched || (iMouseButton & (Mouse::ButtonBase | Mouse::WheelBase))) {
		if(!keyTouched && !bMouseAttack) {
			bMouseAttack=!bMouseAttack;
			return NULL;
		}

		TextWidget * textWidget = static_cast<TextWidget *>(m_selected);

		if(_bValidateTest) {
			if(textWidget->m_isKeybind && textWidget->m_keybindAction == CONTROLS_CUST_ACTION) {
				bool bOk=true;

				if((iMouseButton & Mouse::ButtonBase) && !(iMouseButton & Mouse::WheelBase)) {
					bOk=false;
				} else {
					for(int buttonId = Mouse::ButtonBase; buttonId < Mouse::ButtonMax; buttonId++) {
						if(keyId == buttonId) {
							bOk=false;
							break;
						}
					}
				}

				if(bOk)
					return NULL;
			}
		}

		std::string pText;
		if(iMouseButton & (Mouse::ButtonBase | Mouse::WheelBase)) {
			if(pInputKeyId)
				*pInputKeyId = iMouseButton;
			pText = GInput->getKeyName(iMouseButton, true);
		} else {
			pText = GInput->getKeyName(keyId, true);
		}

		if(pText.empty()) {
			pText = "---";
		}

		textWidget->lColorHighlight = textWidget->lOldColor;
		textWidget->eState = GETTOUCH;
		textWidget->SetText(pText);

		float iDx = m_selected->m_rect.width();

		if(m_selected->ePlace) {
			m_selected->m_rect.left = (m_rect.width() - iDx) / 2.f;

			if(m_selected->m_rect.left < 0) {
				m_selected->m_rect.left=0;
			}
		}

		m_selected->m_rect.right=m_selected->m_rect.left+iDx;

		m_selected=NULL;
		bEdit=false;
		bMouseAttack=false;

		return textWidget;
	}

	return NULL;
}

MENUSTATE MenuPage::Update(Vec2f pos) {

	m_children.Move(m_pos - m_oldPos);
	
	m_oldPos.x=m_pos.x;
	m_oldPos.y=m_pos.y;
	m_pos = pos;
	
	// Check if mouse over
	if(!bEdit) {
		m_selected=NULL;
		Widget * widget = m_children.getAtPos(Vec2f(GInput->getMousePosition()));
		
		if(widget) {
			m_selected = widget;
			
			if(GInput->getMouseButtonDoubleClick(Mouse::Button_0)) {
				MENUSTATE e = m_selected->m_targetMenu;
				bEdit = m_selected->OnMouseDoubleClick();
				
				if(m_selected->m_id == BUTTON_MENUEDITQUEST_LOAD)
					return MAIN;
				
				if(bEdit)
					return m_selected->m_targetMenu;
				
				return e;
			}
			
			if(GInput->getMouseButton(Mouse::Button_0)) {
				MENUSTATE e = m_selected->m_targetMenu;
				bEdit = m_selected->OnMouseClick();
				return e;
			} else {
				m_selected->EmptyFunction();
			}
		}
	} else {
		if(!m_selected) {
			Widget * widget = m_children.getAtPos(Vec2f(GInput->getMousePosition()));
			
			if(widget) {
				m_selected = widget;
				
				if(GInput->getMouseButtonDoubleClick(Mouse::Button_0)) {
					bEdit = m_selected->OnMouseDoubleClick();
					
					if(bEdit)
						return m_selected->m_targetMenu;
				}
			}
		}
	}
	
	return NOP;
}

MENUSTATE MenuPage::checkShortcuts() {

	if(!bEdit) {
		
		{Widget * w; BOOST_FOREACH(w, m_children.m_widgets) {
			arx_assert(w);
			
			if(w->m_shortcut != ActionKey::UNUSED) {
				if(GInput->isKeyPressedNowUnPressed(w->m_shortcut)) {
					if(m_disableShortcuts) {
						m_disableShortcuts = false;
						break;
					}
					bEdit = w->OnMouseClick();
					m_selected = w;
					return w->m_targetMenu;
				}
			}
		}}
	}
	return NOP;
}

void MenuPage::Render() {

	if(bNoMenu)
		return;
	
	{Widget * w; BOOST_FOREACH(w, m_children.m_widgets) {
		w->Update();
		w->Render();
	}}
	
	//HIGHLIGHT
	if(m_selected) {
		bool bReInit=false;

		m_selected->RenderMouseOver();
		
		{
			static const PlatformDuration m_blinkDuration = PlatformDurationMs(300);
			
			m_blinkTime += g_platformTime.lastFrameDuration();
			if(m_blinkTime > (m_blinkDuration + m_blinkDuration))
				m_blinkTime = PlatformDuration_ZERO;
			
			m_blink = m_blinkTime > m_blinkDuration;
		}
		
		switch(m_selected->eState) {
			case EDIT_TIME:
				UpdateText();
				break;
			case GETTOUCH_TIME: {
				if(m_blink)
					((TextWidget*)m_selected)->lColorHighlight = Color(255, 0, 0);
				else
					((TextWidget*)m_selected)->lColorHighlight = Color(50, 0, 0);
				
				bool keyTouched = GInput->isAnyKeyPressed();
				int keyId = GInput->getKeyPressed();
				
				if( GInput->isKeyPressed(Keyboard::Key_LeftShift)||
					GInput->isKeyPressed(Keyboard::Key_RightShift)||
					GInput->isKeyPressed(Keyboard::Key_LeftCtrl)||
					GInput->isKeyPressed(Keyboard::Key_RightCtrl)||
					GInput->isKeyPressed(Keyboard::Key_LeftAlt)||
					GInput->isKeyPressed(Keyboard::Key_RightAlt)
				) {
					if(!((keyId & INPUT_COMBINATION_MASK )>>16))
						keyTouched = false;
				} else {
					if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_LeftShift)) {
						keyTouched = true;
						keyId = Keyboard::Key_LeftShift;
					}
					
					if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_RightShift)) {
						keyTouched = true;
						keyId = Keyboard::Key_RightShift;
					}
					
					if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_LeftCtrl)) {
						keyTouched = true;
						keyId = Keyboard::Key_LeftCtrl;
					}
					
					if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_RightCtrl)) {
						keyTouched = true;
						keyId = Keyboard::Key_RightCtrl;
					}
					
					if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_LeftAlt)) {
						keyTouched = true;
						keyId = Keyboard::Key_LeftAlt;
					}
					
					if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_RightAlt)) {
						keyTouched = true;
						keyId = Keyboard::Key_RightAlt;
					}
				}
				
				InputKeyId inputKeyId;
				TextWidget * widget = GetTouch(keyTouched, keyId, &inputKeyId, true);
				
				if(widget) {
					if(!bEdit) {
						if(widget->m_isKeybind) {
							if(inputKeyId == Keyboard::Key_Escape) {
								inputKeyId = ActionKey::UNUSED;
								m_disableShortcuts = true;
							}
							config.setActionKey(widget->m_keybindAction, widget->m_keybindIndex, inputKeyId);
						}
					}
					bReInit = true;
				}
			}
			break;
			default:
			{
				if(GInput->getMouseButtonNowPressed(Mouse::Button_0)) {
					Widget * widget = m_children.GetZoneWithID(BUTTON_MENUOPTIONS_CONTROLS_CUST_DEFAULT);
					
					if(widget == m_selected) {
						config.setDefaultActionKeys();
						bReInit=true;
					}
				}
			}
			break;
		}
		
		if(bReInit) {
			ReInitActionKey();
			bMouseAttack=false;
		}
	}
}

void MenuPage::drawDebug() {
	Rectf rect = Rectf(Vec2f(m_pos.x, m_pos.y), m_rect.width(), m_rect.height());
	drawLineRectangle(rect, 0.f, Color::green);
	
	m_children.drawDebug();
}

void MenuPage::ReInitActionKey() {
	
	{Widget * w; BOOST_FOREACH(w, m_children.m_widgets) {
		if(w->type() == WidgetType_Panel) {
			PanelWidget * p = static_cast<PanelWidget *>(w);
			
			{Widget * c; BOOST_FOREACH(c, p->m_children) {
				if(c->type() == WidgetType_Text) {
					TextWidget * t = static_cast<TextWidget *>(c);
					
					if(t->m_isKeybind) {
						m_selected = t;
						GetTouch(true, config.actions[t->m_keybindAction].key[t->m_keybindIndex], NULL, false);
					}
				}
			}}
		}
	}}
}


void Menu2_Open() {
	if(pMenuCursor) {
		pMenuCursor->reset();
	}
}

void Menu2_Close() {
	
	ARXmenu.currentmode = AMCM_OFF;
	
	delete pWindowMenu, pWindowMenu = NULL;
	delete mainMenu, mainMenu = NULL;
	delete pMenuCursor, pMenuCursor = NULL;
}

void MenuReInitAll() {
	
	if(!mainMenu)
		return;
	
	mainMenu->bReInitAll=true;
}
