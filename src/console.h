#pragma once

#include "float.hpp"
#include "mstring.h"
#include "vector2di.h"
#include "keyboard.h"

#include <list>
#include <stack>
#include <string>
#include <memory>

struct tokenizer;
struct color32;
struct mString;
struct PanelQuad;
struct ConsoleCommand;
struct ConsoleVariable;

struct Console {
    bool m_visible;
    char oldCurrent[256];
    char current[256];

    std::list<mString> m_log;
    std::list<mString> m_command_log;
    bool field_220;
    float field_224;
    float m_height = 240.0f;
    uint32_t lineNumber;
    uint32_t field_230;
    int field_234;
    int cmdLogNumber;
    std::string field_23C;
    PanelQuad *field_248;
    bool field_24C;
    bool field_24D;
    bool field_24E;
    bool field_24F;

    Console();

    void * operator new(size_t size);

    void operator delete(void *ptr, size_t size);

    bool StrnCopy(const char *src, char *dest, int *a3);

    void exec_frame_advance(Float a2);

    virtual ~Console();

    virtual void getMatchingCmds(const char *a2, std::list<mString> &cmds);

    virtual void partialCompleteCmd(char *a1, std::list<mString> &list);

    virtual void hide();

    /*virtual*/ void render();

    virtual void exec(const mString &a2);

    virtual void addToLog(const char *arg4, ...);

    virtual void processCommand(const char *a2, bool is_log);

    virtual void addToCommandLog(const char *a1);

    virtual ConsoleCommand *getCommand(const std::string &a1);

    virtual ConsoleVariable *getVariable(const std::string &a1);

    virtual bool isVisible() const;

    virtual void show();

    /* virtual */ void frame_advance(Float a2);

    /* virtual */ void handle_event(KeyEvent a2, Key_Axes a3, void *a4);

    /* virtual */ void handle_char(char a2, void *);

    /* virtual */ void setHeight(Float a2);

    /* virtual */ float getHeight();

    /* virtual */ void setRenderCursor(bool a2);
};

extern void _kbevcb(KeyEvent a1, Key_Axes a2);

extern void _kbchcb(char a1);

extern char KB_register_event_callback(void (*a1)(KeyEvent, Key_Axes, void *), void *a2);

extern void render_console_text(const mString &a1, vector2di a2, const color32 &a4);

extern std::unique_ptr<Console> g_console;

extern std::stack<tokenizer *> s_exec_tok_stack;

extern float s_exec_tick;

extern void console_patch();
