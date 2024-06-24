#pragma once

#include <list>
#include <string>

struct ConsoleVariable {
    char field_4[32];

    ConsoleVariable();

    virtual ~ConsoleVariable() = default;

    virtual void setValue(const std::string &);

    virtual void setValue(const std::string &, const std::string &);

    virtual std::string getValue();

    virtual const char *helpText() {
        return "No help available.";
    }

    void setName(const std::string &pName);

    bool match(std::string a2) const;

    std::string getName() const;
};

extern std::list<ConsoleVariable *> *g_console_vars;

struct ConsoleHeightVariable : ConsoleVariable {
    ConsoleHeightVariable();

    using ConsoleVariable::setValue;
    virtual void setValue(const std::string &a2) override;

    virtual std::string getValue() override;

    virtual const char *helpText() override {
        return "Height of the console in pixels";
    }
};

struct HealthVariable : ConsoleVariable {
    HealthVariable();

    using ConsoleVariable::setValue;
    void setValue(const std::string &arg0, const std::string &a1) override;

    void setValue(const std::string &arg0) override;

    virtual ~HealthVariable() = default;
};

struct RenderFramerateVariable : ConsoleVariable {
    RenderFramerateVariable();

    using ConsoleVariable::setValue;
    void setValue(const std::string &a1) override;

    std::string getValue() override;

    const char *helpText() override {
        return "Render frames per second";
    }
};

struct RenderInterfaceVariable : ConsoleVariable {
    RenderInterfaceVariable();

    using ConsoleVariable::setValue;
    void setValue(const std::string &a1) override;

    std::string getValue() override;

    const char *helpText() override {
        return "Render in-game user interface";
    }
};

struct ProjZoomVariable : ConsoleVariable {
    ProjZoomVariable();

    using ConsoleVariable::setValue;
    void setValue(const std::string &a2) override;

    std::string getValue() override;

    const char *helpText() override {
        return "FOV factor";
    }
};

struct DifficultyVariable : ConsoleVariable {
    DifficultyVariable();

    using ConsoleVariable::setValue;
    void setValue(const std::string &a2) override;

    std::string getValue() override;

    const char *helpText() override {
        return "Difficulty level (0=bleep, 1=ez, 2=norm, 3=hero, 4=super hero)";
    }
};

struct DisableOcclusionCullingVariable : ConsoleVariable {

    DisableOcclusionCullingVariable() {
        setName("disable_occlusion_culling");
    }

    using ConsoleVariable::setValue;
    virtual void setValue(const std::string &a2) override;

    virtual std::string getValue() override;

    const char *helpText() override {
        return "1 = disable entity culling, 2 = disable terrain culling, 3 = disable both";
    }
};
