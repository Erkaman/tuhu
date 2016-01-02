#pragma once

#include <string>

class GuiListener {

public:
    virtual void TranslationAccepted()=0;
    virtual void RotationAccepted()=0;
    virtual void ScaleAccepted()=0;

    virtual void ModelAdded(const std::string& filename)=0;

    virtual void CursorSizeChanged()=0;

    virtual void Duplicate()=0;
    virtual void Delete()=0;

};
