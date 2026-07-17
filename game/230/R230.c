#include <common.h>

struct OverlayRDATA_230 R230 = {.overlayTag = 0x10,

                                .jmpPtrs_Title_MenuUpdate = {0x800abdf0, 0x800abe30, 0x800abe54, 0x800abe80, 0x800abea4, 0x800abfa0},

                                .s_title = "title",

                                .packedDefaultCharacterIDWords = {0x03020100, 0x07060504},

                                .s_4 = "4",
                                .s_3 = "3",
                                .s_2 = "2",
                                .s_1 = "1",

                                .jmpPtrs_Characters_MenuProc = {0x800aeacc, 0x800aeacc, 0x800ae974, 0x800ae9fc, 0x800aea88, 0x800aea88},

                                .s_loaded_ghost_data = "loaded ghost data",

                                .jmpPtrs_Battle_MenuProc =
                                    {
                                        0x800b1e28,
                                        0x800b1e28,
                                        0x800b2124,
                                        0x800b1e3c,
                                        0x800b1e3c,
                                        0x800b1e98,
                                        0x800b2124,
                                        0x800b2124,
                                        0x800b2124,
                                        0x800b2124,
                                        0x800b1e28,
                                    },

                                .s_teststr1 = "\\TEST.STR;1",

                                .ptr_MM_TrackSelect_boolTrackOpen = 0x800b4058};
