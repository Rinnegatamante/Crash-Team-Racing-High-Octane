enum NativeHighScoreOnlineState
{
    MM_HIGHSCORE_ONLINE_BROWSING = 0,
    MM_HIGHSCORE_ONLINE_GHOST_MENU,
    MM_HIGHSCORE_ONLINE_DOWNLOADING,
};

static struct MenuRow s_onlineGhostRows[] =
{
    {NATIVE_MENU_STRING_WATCH_GHOST, 1, 1, 0, 0},
    {LNG_TIME_TRIAL_EXIT, 0, 0, 1, 1},
    {RECTMENU_STRING_NONE},
};

static struct RectMenu s_onlineGhostMenu =
{
    .stringIndexTitle = RECTMENU_STRING_NONE,
    .posX_curr = 0x17c,
    .posY_curr = 0xaf,
    .state = USE_SMALL_FONT | CENTER_ON_X,
    .rows = s_onlineGhostRows,
};

static int s_onlineHighScoreState;
static int s_onlineSelectedRace;
static u16 s_onlineGhostTrackId;
static u16 s_onlineGhostCharacterId;

static int MM_HighScore_OnlineGetTrack(u16 trackIndex, struct NativeLeaderboardTrack *track)
{
    if (trackIndex > MM_HIGHSCORE_LAST_ARCADE_TRACK)
    {
        return 0;
    }

    return NativeLeaderboard_CopyTrack(D230.arcadeTracks[trackIndex].levID, track);
}

static void MM_HighScore_OnlineClampSelection(void)
{
    struct NativeLeaderboardTrack track;
    if (!MM_HighScore_OnlineGetTrack(D230.highScoreSelection.currentTrack, &track) || (track.raceCount <= 0))
    {
        s_onlineSelectedRace = -1;
        return;
    }

    if (s_onlineSelectedRace < 0)
    {
        s_onlineSelectedRace = 0;
    }
    if (s_onlineSelectedRace >= track.raceCount)
    {
        s_onlineSelectedRace = track.raceCount - 1;
    }
}

static void MM_HighScore_OnlineDraw(u16 trackIndex, s16 offsetX)
{
    struct GameTracker *gGT = sdata->gGT;
    s16 levelID = D230.arcadeTracks[trackIndex].levID;
    struct NativeLeaderboardTrack track;
    memset(&track, 0, sizeof(track));
    MM_HighScore_OnlineGetTrack(trackIndex, &track);

    s16 lineWidth = DecalFont_GetLineWidth(sdata->lngStrings[data.metaDataLEV[levelID].name_LNG], FONT_BIG) >> 1;
    s16 numColor = ((sdata->frameCounter & MM_HIGHSCORE_FLASH_TIMER_BIT) == 0) ? RED : ORANGE;
    u32 *colorPtr = data.ptrColor[numColor];
    struct Icon **iconPtrArray = ICONGROUP_GETICONS(gGT->iconGroup[MM_HIGHSCORE_ARROW_ICON_GROUP]);
    const struct TransitionMeta *titleMeta = &D230.transitionMeta_HighScores[MM_HIGHSCORE_TITLE_META_INDEX];
    const struct TransitionMeta *bestTrackMeta = &D230.transitionMeta_HighScores[MM_HIGHSCORE_BEST_TRACK_META_INDEX];
    const struct TransitionMeta *bestLapLabelMeta = &D230.transitionMeta_HighScores[MM_HIGHSCORE_BEST_LAP_LABEL_META_INDEX];
    const struct TransitionMeta *bestLapEntryMeta = &D230.transitionMeta_HighScores[MM_HIGHSCORE_BEST_LAP_ENTRY_META_INDEX];
    Color iconColor = D230.highscore_iconColor;

    DecalHUD_Arrow2D(iconPtrArray[MM_HIGHSCORE_ARROW_ICON_ID], titleMeta->currX + offsetX - lineWidth + MM_HIGHSCORE_ARROW_LEFT_X_OFFSET,
                     titleMeta->currY + MM_HIGHSCORE_ARROW_Y_OFFSET, &gGT->backBuffer->primMem, gGT->pushBuffer_UI.ptrOT,
                     colorPtr[0], colorPtr[1], colorPtr[2], colorPtr[3], 0, MM_HIGHSCORE_ARROW_SCALE, MM_HIGHSCORE_ARROW_LEFT_ROTATION);
    DecalHUD_Arrow2D(iconPtrArray[MM_HIGHSCORE_ARROW_ICON_ID], titleMeta->currX + offsetX + lineWidth + MM_HIGHSCORE_ARROW_RIGHT_X_OFFSET,
                     titleMeta->currY + MM_HIGHSCORE_ARROW_Y_OFFSET, &gGT->backBuffer->primMem, gGT->pushBuffer_UI.ptrOT,
                     colorPtr[0], colorPtr[1], colorPtr[2], colorPtr[3], 0, MM_HIGHSCORE_ARROW_SCALE, 0);

    DecalFont_DrawLine(sdata->lngStrings[data.metaDataLEV[levelID].name_LNG], titleMeta->currX + offsetX + MM_HIGHSCORE_TITLE_X_OFFSET,
                       titleMeta->currY + MM_HIGHSCORE_TITLE_Y_OFFSET, FONT_BIG, JUSTIFY_CENTER);
    MM_HighScore_Text3D(sdata->lngStrings[LNG_BEST_TRACK_TIMES], bestTrackMeta->currX + offsetX + MM_HIGHSCORE_BEST_TRACK_LABEL_X_OFFSET,
                        bestTrackMeta->currY + MM_HIGHSCORE_BEST_TRACK_LABEL_Y_OFFSET, FONT_SMALL, 0);
    MM_HighScore_Text3D(sdata->lngStrings[LNG_BEST_LAP_TIME], bestLapLabelMeta->currX + offsetX + MM_HIGHSCORE_BEST_LAP_LABEL_X_OFFSET,
                        bestLapLabelMeta->currY + MM_HIGHSCORE_BEST_LAP_LABEL_Y_OFFSET, FONT_SMALL, 0);

    if (track.hasLap)
    {
        struct NativeLeaderboardEntry *lap = &track.lap;
        MM_HighScore_Text3D(lap->nickname, bestLapEntryMeta->currX + offsetX + MM_HIGHSCORE_BEST_LAP_TEXT_X_OFFSET,
                            bestLapEntryMeta->currY + MM_HIGHSCORE_BEST_LAP_NAME_Y_OFFSET, FONT_BIG,
                            lap->characterId + MM_HIGHSCORE_DRIVER_COLOR_OFFSET);
        MM_HighScore_Text3D(RECTMENU_DrawTime(lap->timeMs), bestLapEntryMeta->currX + offsetX + MM_HIGHSCORE_BEST_LAP_TEXT_X_OFFSET,
                            bestLapEntryMeta->currX + MM_HIGHSCORE_BEST_LAP_TIME_Y_OFFSET, FONT_SMALL, 0);
        RECTMENU_DrawPolyGT4(gGT->ptrIcons[data.MetaDataCharacters[lap->characterId].iconID],
                             bestLapEntryMeta->currX + offsetX + MM_HIGHSCORE_BEST_LAP_ICON_X_OFFSET,
                             bestLapEntryMeta->currY + MM_HIGHSCORE_BEST_LAP_ICON_Y_OFFSET, &gGT->backBuffer->primMem,
                             gGT->pushBuffer_UI.ptrOT, iconColor.self, iconColor.self, iconColor.self, iconColor.self,
                             MM_HIGHSCORE_ICON_TRANSPARENCY, MM_HIGHSCORE_ICON_SCALE);
    }

    for (int row = 0; row < track.raceCount && row < MM_HIGHSCORE_VISIBLE_SCORE_ROWS; row++)
    {
        struct NativeLeaderboardEntry *entry = &track.race[row];
        int metaIndex = row + MM_HIGHSCORE_FIRST_VISIBLE_META_INDEX;

        RECTMENU_DrawPolyGT4(gGT->ptrIcons[data.MetaDataCharacters[entry->characterId].iconID],
                             D230.transitionMeta_HighScores[metaIndex].currX + offsetX + MM_HIGHSCORE_SCORE_ICON_X_OFFSET,
                             D230.transitionMeta_HighScores[metaIndex].currY + row * MM_HIGHSCORE_SCORE_ROW_Y_STEP + MM_HIGHSCORE_SCORE_NAME_Y_OFFSET,
                             &gGT->backBuffer->primMem, gGT->pushBuffer_UI.ptrOT, iconColor.self, iconColor.self, iconColor.self, iconColor.self,
                             MM_HIGHSCORE_ICON_TRANSPARENCY, MM_HIGHSCORE_ICON_SCALE);
        MM_HighScore_Text3D(entry->nickname, D230.transitionMeta_HighScores[metaIndex].currX + offsetX + MM_HIGHSCORE_SCORE_NAME_X_OFFSET,
                            D230.transitionMeta_HighScores[metaIndex].currY + row * MM_HIGHSCORE_SCORE_ROW_Y_STEP + MM_HIGHSCORE_SCORE_NAME_Y_OFFSET,
                            FONT_BIG, entry->characterId + MM_HIGHSCORE_DRIVER_COLOR_OFFSET);
        MM_HighScore_Text3D(RECTMENU_DrawTime(entry->timeMs),
                            D230.transitionMeta_HighScores[metaIndex].currX + offsetX + MM_HIGHSCORE_SCORE_NAME_X_OFFSET,
                            D230.transitionMeta_HighScores[metaIndex].currY + row * MM_HIGHSCORE_SCORE_ROW_Y_STEP + MM_HIGHSCORE_SCORE_TIME_Y_OFFSET,
                            FONT_SMALL, 0);

        // The PS1 OT is LIFO: submit the highlight after the row content so it renders behind it.
        if ((trackIndex == D230.highScoreSelection.currentTrack) && (D230.highScoreTransition.trackFrame == 0) &&
            (s_onlineHighScoreState == MM_HIGHSCORE_ONLINE_BROWSING) && (row == s_onlineSelectedRace))
        {
            RECT highlight;
            highlight.x = D230.transitionMeta_HighScores[metaIndex].currX + offsetX + MM_HIGHSCORE_SCORE_ICON_X_OFFSET - 8;
            highlight.y = D230.transitionMeta_HighScores[metaIndex].currY + row * MM_HIGHSCORE_SCORE_ROW_Y_STEP + MM_HIGHSCORE_SCORE_NAME_Y_OFFSET - 2;
            highlight.w = 0xe8;
            highlight.h = MM_HIGHSCORE_SCORE_ROW_Y_STEP;
            CTR_Box_DrawClearBox(&highlight, &sdata->menuRowHighlight_Normal, TRANS_50_DECAL, gGT->backBuffer->otMem.uiOT);
        }
    }

    RECT videoBox;
    videoBox.w = MM_HIGHSCORE_VIDEO_BOX_W;
    videoBox.h = MM_HIGHSCORE_VIDEO_BOX_H;
    videoBox.x = D230.transitionMeta_HighScores[MM_HIGHSCORE_VIDEO_META_INDEX].currX + offsetX + MM_HIGHSCORE_VIDEO_BOX_X_OFFSET;
    videoBox.y = D230.transitionMeta_HighScores[MM_HIGHSCORE_VIDEO_META_INDEX].currY + MM_HIGHSCORE_VIDEO_BOX_Y_OFFSET;
    MM_TrackSelect_Video_Draw(&videoBox, &D230.arcadeTracks[0], trackIndex, (D230.highScoreTransition.state == EXITING_MENU), 0);
}

static void MM_HighScore_OnlineStartGhostReplay(void)
{
    void *ghostData = NULL;
    int ghostSize = 0;
    if (!NativeLeaderboard_TakeGhost(&ghostData, &ghostSize)) return;

    b32 valid = NativeGhostInput_LoadSerializedGhost(ghostData, ghostSize, s_onlineGhostTrackId, s_onlineGhostCharacterId);
    free(ghostData);
    if (!valid)
    {
        OtherFX_Play(5, 1);
        s_onlineHighScoreState = MM_HIGHSCORE_ONLINE_GHOST_MENU;
        return;
    }

    struct GameTracker *gGT = sdata->gGT;
    gNativeGhostReplayMode = 1;
    gNativeOnlineLeaderboardMode = 0;
    data.characterIDs[0] = s_onlineGhostCharacterId;
    gGT->numPlyrNextGame = 1;
    gGT->gameMode1 &= ~(BATTLE_MODE | RELIC_RACE | ADVENTURE_MODE | ADVENTURE_ARENA | ARCADE_MODE | ADVENTURE_CUP);
    gGT->gameMode1 |= TIME_TRIAL;
    gGT->gameMode2 &= ~(CUP_ANY_KIND | CHEAT_WUMPA | CHEAT_MASK | CHEAT_TURBO | CHEAT_ENGINE | CHEAT_BOMBS);
    gGT->currLEV = s_onlineGhostTrackId;
    sdata->boolReplayHumanGhost = 0;
    MM_Title_KillThread();
    sdata->ptrDesiredMenu = QueueLoadTrack_GetMenuPtr();
}

static void MM_HighScore_OnlineInit(void)
{
    s_onlineHighScoreState = MM_HIGHSCORE_ONLINE_BROWSING;
    s_onlineSelectedRace = 0;
    s_onlineGhostMenu.rowSelected = 0;
    D230.highScoreTransition.state = ENTERING_MENU;
    D230.highScoreTransition.mainFrame = MM_HIGHSCORE_MAIN_TRANSITION_MAX_FRAME;
    D230.highScoreTransition.trackFrame = 0;
    D230.highScoreTransition.rowFrame = 0;
    D230.highScoreSelection.targetRow = 0;
    D230.highScoreSelection.currentRow = 0;
    MM_TrackSelect_Video_SetDefaults();
    NativeLeaderboard_RequestRefresh();
}

static void MM_HighScore_OnlineHandleBrowsingInput(void)
{
    u32 buttons = sdata->buttonTapPerPlayer[0];
    struct NativeLeaderboardTrack track;
    memset(&track, 0, sizeof(track));
    MM_HighScore_OnlineGetTrack(D230.highScoreSelection.currentTrack, &track);
    MM_HighScore_OnlineClampSelection();

    if ((buttons & (BTN_TRIANGLE | BTN_SQUARE_one)) != 0)
    {
        OtherFX_Play(2, 1);
        D230.highScoreTransition.state = EXITING_MENU;
        return;
    }

    if ((buttons & BTN_LEFT) != 0)
    {
        OtherFX_Play(0, 1);
        D230.highScoreTransition.pendingHorizontalMove = -1;
        D230.highScoreSelection.targetTrack = D230.highScoreSelection.currentTrack - 1;
        if (D230.highScoreSelection.targetTrack < 0) D230.highScoreSelection.targetTrack = MM_HIGHSCORE_LAST_ARCADE_TRACK;
        return;
    }
    if ((buttons & BTN_RIGHT) != 0)
    {
        OtherFX_Play(0, 1);
        D230.highScoreTransition.pendingHorizontalMove = 1;
        D230.highScoreSelection.targetTrack = D230.highScoreSelection.currentTrack + 1;
        if (D230.highScoreSelection.targetTrack > MM_HIGHSCORE_LAST_ARCADE_TRACK) D230.highScoreSelection.targetTrack = 0;
        return;
    }

    if (track.raceCount > 0)
    {
        if ((buttons & BTN_UP) != 0)
        {
            OtherFX_Play(0, 1);
            s_onlineSelectedRace--;
            if (s_onlineSelectedRace < 0) s_onlineSelectedRace = track.raceCount - 1;
            return;
        }
        if ((buttons & BTN_DOWN) != 0)
        {
            OtherFX_Play(0, 1);
            s_onlineSelectedRace++;
            if (s_onlineSelectedRace >= track.raceCount) s_onlineSelectedRace = 0;
            return;
        }
    }

    if ((buttons & (BTN_CROSS_one | BTN_CIRCLE)) != 0)
    {
        if ((s_onlineSelectedRace < 0) || (s_onlineSelectedRace >= track.raceCount) || !track.race[s_onlineSelectedRace].hasGhost)
        {
            OtherFX_Play(5, 1);
            return;
        }

        OtherFX_Play(1, 1);
        s_onlineGhostMenu.rowSelected = 0;
        s_onlineHighScoreState = MM_HIGHSCORE_ONLINE_GHOST_MENU;
    }
}

static void MM_HighScore_OnlineHandleGhostMenuInput(void)
{
    u32 buttons = sdata->buttonTapPerPlayer[0];
    if ((buttons & (BTN_TRIANGLE | BTN_SQUARE_one)) != 0)
    {
        OtherFX_Play(2, 1);
        s_onlineHighScoreState = MM_HIGHSCORE_ONLINE_BROWSING;
        return;
    }

    if ((buttons & (BTN_UP | BTN_DOWN)) != 0)
    {
        OtherFX_Play(0, 1);
        s_onlineGhostMenu.rowSelected ^= 1;
        return;
    }

    if ((buttons & (BTN_CROSS_one | BTN_CIRCLE)) == 0) return;
    if (s_onlineGhostMenu.rowSelected == 1)
    {
        OtherFX_Play(2, 1);
        s_onlineHighScoreState = MM_HIGHSCORE_ONLINE_BROWSING;
        return;
    }

    struct NativeLeaderboardTrack track;
    if (!MM_HighScore_OnlineGetTrack(D230.highScoreSelection.currentTrack, &track) ||
        (s_onlineSelectedRace < 0) || (s_onlineSelectedRace >= track.raceCount))
    {
        OtherFX_Play(5, 1);
        s_onlineHighScoreState = MM_HIGHSCORE_ONLINE_BROWSING;
        return;
    }

    struct NativeLeaderboardEntry *entry = &track.race[s_onlineSelectedRace];
    if (!entry->hasGhost || !NativeLeaderboard_RequestGhost(entry->recordId))
    {
        OtherFX_Play(5, 1);
        return;
    }

    OtherFX_Play(1, 1);
    s_onlineGhostTrackId = D230.arcadeTracks[D230.highScoreSelection.currentTrack].levID;
    s_onlineGhostCharacterId = entry->characterId;
    s_onlineHighScoreState = MM_HIGHSCORE_ONLINE_DOWNLOADING;
    RaceFlag_ResetTextAnim();
}

static void MM_HighScore_OnlineMenuProc(void)
{
    s16 nextFrameCount;
    b32 videoResetRequested = false;

    if (!NativeLeaderboard_HasCache() && !NativeLeaderboard_IsRefreshing()) NativeLeaderboard_RequestRefresh();

    if (s_onlineHighScoreState == MM_HIGHSCORE_ONLINE_DOWNLOADING)
    {
        int ghostState = NativeLeaderboard_GetGhostState();
        if (ghostState == NATIVE_LEADERBOARD_TRANSFER_READY)
        {
            MM_HighScore_OnlineStartGhostReplay();
            return;
        }
        if (ghostState == NATIVE_LEADERBOARD_TRANSFER_FAILED)
        {
            OtherFX_Play(5, 1);
            s_onlineHighScoreState = MM_HIGHSCORE_ONLINE_GHOST_MENU;
        }
    }

    if (D230.highScoreTransition.state != IN_MENU)
    {
        nextFrameCount = D230.highScoreTransition.mainFrame;
        if (D230.highScoreTransition.state == ENTERING_MENU)
        {
            MM_TransitionInOut(D230.transitionMeta_HighScores, D230.highScoreTransition.mainFrame, MM_HIGHSCORE_SLIDE_TRANSITION_FRAMES);
            nextFrameCount--;
            if (D230.highScoreTransition.mainFrame == 0)
            {
                D230.highScoreTransition.state = IN_MENU;
                nextFrameCount = 0;
            }
        }
        else if ((D230.highScoreTransition.state == EXITING_MENU) && (D230.highScoreTransition.trackFrame == 0))
        {
            MM_TransitionInOut(D230.transitionMeta_HighScores, D230.highScoreTransition.mainFrame, MM_HIGHSCORE_SLIDE_TRANSITION_FRAMES);
            D230.highScoreTransition.mainFrame++;
            nextFrameCount = D230.highScoreTransition.mainFrame;
            if (D230.highScoreTransition.mainFrame > MM_HIGHSCORE_MAIN_TRANSITION_MAX_FRAME)
            {
                gNativeOnlineLeaderboardMode = 0;
                MM_JumpTo_Title_Returning();
                return;
            }
        }
        D230.highScoreTransition.mainFrame = nextFrameCount;
    }

    if ((D230.highScoreTransition.state == IN_MENU) && (D230.highScoreTransition.trackFrame == 0) &&
        (s_onlineHighScoreState != MM_HIGHSCORE_ONLINE_DOWNLOADING))
    {
        if (s_onlineHighScoreState == MM_HIGHSCORE_ONLINE_GHOST_MENU) MM_HighScore_OnlineHandleGhostMenuInput();
        else MM_HighScore_OnlineHandleBrowsingInput();
    }

    if (D230.highScoreTransition.trackFrame == 0)
    {
        if (D230.highScoreSelection.currentTrack != D230.highScoreSelection.targetTrack)
        {
            D230.highScoreTransition.trackFrame = MM_HIGHSCORE_SLIDE_TRANSITION_FRAMES;
            D230.highScoreTransition.activeHorizontalMove = D230.highScoreTransition.pendingHorizontalMove;
            s_onlineSelectedRace = 0;
            videoResetRequested = true;
        }
    }
    else
    {
        u8 reachedTarget = D230.highScoreTransition.trackFrame == 1;
        D230.highScoreTransition.trackFrame--;
        if (reachedTarget)
        {
            D230.highScoreSelection.currentTrack = D230.highScoreSelection.targetTrack;
            MM_HighScore_OnlineClampSelection();
        }
    }

    b32 videoState = videoResetRequested || (D230.highScoreTransition.trackFrame != 0) ||
                     (D230.highScoreTransition.state == EXITING_MENU) ||
                     (s_onlineHighScoreState == MM_HIGHSCORE_ONLINE_DOWNLOADING);
    MM_TrackSelect_Video_State(videoState);

    if (s_onlineHighScoreState == MM_HIGHSCORE_ONLINE_DOWNLOADING)
    {
        RaceFlag_DrawLoadingString();
        return;
    }

    if (!NativeLeaderboard_HasCache())
    {
        RaceFlag_DrawLoadingString();
        return;
    }

    s32 currOffsetX = 0;
    if (D230.highScoreTransition.trackFrame != 0)
    {
        currOffsetX = (MM_HIGHSCORE_SLIDE_TRANSITION_FRAMES - D230.highScoreTransition.trackFrame) *
                      D230.highScoreTransition.activeHorizontalMove * MM_HIGHSCORE_TRACK_SLIDE_STEP_X;
    }
    if ((currOffsetX != -MM_HIGHSCORE_OFFSCREEN_X) && (currOffsetX != MM_HIGHSCORE_OFFSCREEN_X))
    {
        MM_HighScore_OnlineDraw(D230.highScoreSelection.currentTrack, (s16)currOffsetX);
    }

    if (D230.highScoreTransition.trackFrame != 0)
    {
        s32 nextOffsetX = D230.highScoreTransition.trackFrame * -MM_HIGHSCORE_TRACK_SLIDE_STEP_X *
                          D230.highScoreTransition.activeHorizontalMove;
        if ((nextOffsetX != -MM_HIGHSCORE_OFFSCREEN_X) && (nextOffsetX != MM_HIGHSCORE_OFFSCREEN_X))
        {
            MM_HighScore_OnlineDraw(D230.highScoreSelection.targetTrack, (s16)nextOffsetX);
        }
    }

    RECT wipeRect;
    const struct TransitionMeta *titleMeta = &D230.transitionMeta_HighScores[MM_HIGHSCORE_TITLE_META_INDEX];
    wipeRect.w = MM_HIGHSCORE_WIPE_RECT_W;
    wipeRect.h = MM_HIGHSCORE_WIPE_RECT_H;
    wipeRect.x = titleMeta->currX + MM_HIGHSCORE_WIPE_RECT_X_OFFSET;
    wipeRect.y = titleMeta->currY + MM_HIGHSCORE_WIPE_RECT_Y_OFFSET;
    RECTMENU_DrawInnerRect(&wipeRect, 0, sdata->gGT->backBuffer->otMem.uiOT);

    if (s_onlineHighScoreState == MM_HIGHSCORE_ONLINE_GHOST_MENU)
    {
        RECTMENU_DrawSelf(&s_onlineGhostMenu, D230.transitionMeta_HighScores[MM_HIGHSCORE_MENU_META_INDEX].currX,
                          D230.transitionMeta_HighScores[MM_HIGHSCORE_MENU_META_INDEX].currY, MM_HIGHSCORE_MENU_WIDTH);
    }
}
