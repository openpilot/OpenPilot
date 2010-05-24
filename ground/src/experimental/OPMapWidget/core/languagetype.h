#ifndef LANGUAGETYPE_H
#define LANGUAGETYPE_H

#include <QString>
#include <QStringList>


class LanguageType
{
public:

    enum Types
    {
        Arabic,
        Bulgarian,
        Bengali,
        Catalan,
        Czech,
        Danish,
        German,
        Greek,
        English,
        EnglishAustralian,
        EnglishGreatBritain,
        Spanish,
        Basque,
        Finnish,
        Filipino,
        French,
        Galician,
        Gujarati,
        Hindi,
        Croatian,
        Hungarian,
        Indonesian,
        Italian,
        Hebrew,
        Japanese,
        Kannada,
        Korean,
        Lithuanian,
        Latvian,
        Malayalam,
        Marathi,
        Dutch,
        NorwegianNynorsk,
        Norwegian,
        Oriya,
        Polish,
        Portuguese,
        PortugueseBrazil,
        PortuguesePortugal,
        Romansch,
        Romanian,
        Russian,
        Slovak,
        Slovenian,
        Serbian,
        Swedish,
        Tamil,
        Telugu,
        Thai,
        Turkish,
        Ukrainian,
        Vietnamese,
        ChineseSimplified,
        ChineseTraditional,
    };


    QString toString(Types type);
    LanguageType();
    ~LanguageType();
private:
    QStringList list;
};

#endif // LANGUAGETYPE_H
