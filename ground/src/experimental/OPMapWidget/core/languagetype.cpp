#include "languagetype.h"


LanguageType::LanguageType()
{
    list
            <<"ar"
            <<"bg"
            <<"bn"
            <<"ca"
            <<"cs"
            <<"da"
            <<"de"
            <<"el"
            <<"en"
            <<"en-AU"
            <<"en-GB"
            <<"es"
            <<"eu"
            <<"fi"
            <<"fil"
            <<"fr"
            <<"gl"
            <<"gu"
            <<"hi"
            <<"hr"
            <<"hu"
            <<"id"
            <<"it"
            <<"iw"
            <<"ja"
            <<"kn"
            <<"ko"
            <<"lt"
            <<"lv"
            <<"ml"
            <<"mr"
            <<"nl"
            <<"nn"
            <<"no"
            <<"or"
            <<"pl"
            <<"pt"
            <<"pt-BR"
            <<"pt-PT"
            <<"rm"
            <<"ro"
            <<"ru"
            <<"sk"
            <<"sl"
            <<"sr"
            <<"sv"
            <<"ta"
            <<"te"
            <<"th"
            <<"tr"
            <<"uk"
            <<"vi"
            <<"zh-CN"
            <<"zh-TW";

}
QString LanguageType::toString(Types type)
{
    return list[type];
}
LanguageType::~LanguageType()
{
    list.clear();
}

