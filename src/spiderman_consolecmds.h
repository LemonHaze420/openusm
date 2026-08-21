#pragma once

#include "consolecmds.h"

struct AmbientLightCommand : ConsoleCommand {
    AmbientLightCommand();

    virtual bool process_cmd(const std::vector<std::string> &a2) override;

    virtual const char *helpText() const override
    {
        return "amb_light <brightness> [or] <r> <g> <b> (all values 0.0f -> 1.0f)";
    }
};

struct SetTimeOfDayCommand : ConsoleCommand {
    SetTimeOfDayCommand();

    virtual bool process_cmd(const std::vector<std::string> &a2) override;

    virtual const char *helpText() const override
    {
        return "tod 0=Day 1=Night 2=Rainy 3=Sunset";
    }
};

struct SetBlendModeCommand : ConsoleCommand {
    SetBlendModeCommand();

    virtual bool process_cmd(const std::vector<std::string> &a2) override;

    virtual const char *helpText() const override
    {
        return "blend [0-8]";
    }
};

struct SetMaterialFeaturesCommand : ConsoleCommand {
    SetMaterialFeaturesCommand();

    virtual bool process_cmd(const std::vector<std::string> &a2) override;

    virtual const char *helpText() const override
    {
        return "mat [0-3]";
    }
};

struct SetCameraCommand : ConsoleCommand {
    SetCameraCommand();

    virtual bool process_cmd(const std::vector<std::string> &a2) override;

    virtual const char *helpText() const override
    {
        return "camera [0-2]";
    }
};
