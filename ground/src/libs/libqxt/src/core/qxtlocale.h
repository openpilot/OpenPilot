/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtCore module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/

#ifndef QXTLOCALE_H
#define QXTLOCALE_H
#include <QLocale>
#include <QObject>
#include <qxtglobal.h>

class QXT_CORE_EXPORT QxtLocale
{
    Q_GADGET
    Q_ENUMS(Currency Continent)
public:

    enum Currency
    {
        NoCurrency = 0,
        Afghani = 1,
        AlgerianDinar = 2,
        ArgentinePeso = 3,
        ArmenianDram = 4,
        ArubanGuilder = 5,
        AustralianDollar = 6,
        AzerbaijanianManat = 7,
        BahamianDollar = 8,
        BahrainiDinar = 9,
        Baht = 10,
        Balboa = 11,
        BarbadosDollar = 12,
        BelarussianRuble = 13,
        BelizeDollar = 14,
        BermudianDollar = 15,
        BolivarFuerte = 16,
        Boliviano = 17,
        BrazilianReal = 18,
        BruneiDollar = 19,
        BulgarianLev = 20,
        BurundiFranc = 21,
        CFAFrancBCEAO = 22,
        CFAFrancBEAC = 23,
        CFPFranc = 24,
        CanadianDollar = 25,
        CapeVerdeEscudo = 26,
        CaymanIslandsDollar = 27,
        ChileanPeso = 28,
        ColombianPeso = 29,
        ComoroFranc = 30,
        ConvertibleMarks = 31,
        CordobaOro = 32,
        CostaRicanColon = 33,
        CroatianKuna = 34,
        CubanPeso = 35,
        CyprusPound = 36,
        CzechKoruna = 37,
        Dalasi = 38,
        DanishKrone = 39,
        Denar = 40,
        DjiboutiFranc = 41,
        Dobra = 42,
        DominicanPeso = 43,
        Dong = 44,
        EastCaribbeanDollar = 45,
        EgyptianPound = 46,
        ElSalvadorColon = 47,
        EthiopianBirr = 48,
        Euro = 49,
        FalklandIslandsPound = 50,
        FijiDollar = 51,
        Forint = 52,
        FrancCongolais = 53,
        GhanaCedi = 54,
        GibraltarPound = 55,
        Gourde = 56,
        Guarani = 57,
        GuineaBissauPeso = 58,
        GuineaFranc = 59,
        GuyanaDollar = 60,
        HongKongDollar = 61,
        Hryvnia = 62,
        IcelandKrona = 63,
        IndianRupee = 64,
        IranianRial = 65,
        IraqiDinar = 66,
        JamaicanDollar = 67,
        JordanianDinar = 68,
        KenyanShilling = 69,
        Kina = 70,
        Kip = 71,
        Kroon = 72,
        KuwaitiDinar = 73,
        Kwanza = 74,
        Kyat = 75,
        Lari = 76,
        LatvianLats = 77,
        LebanesePound = 78,
        Lek = 79,
        Lempira = 80,
        Leone = 81,
        LiberianDollar = 82,
        LibyanDinar = 83,
        Lilangeni = 84,
        LithuanianLitas = 85,
        Loti = 86,
        MalagasyAriary = 87,
        MalawiKwacha = 88,
        MalaysianRinggit = 89,
        MalteseLira = 90,
        Manat = 91,
        MauritiusRupee = 92,
        Metical = 93,
        MexicanPeso = 94,
        MexicanUnidadDeInversion = 95,
        MoldovanLeu = 96,
        MoroccanDirham = 97,
        Mvdol = 98,
        Naira = 99,
        Nakfa = 100,
        NamibiaDollar = 101,
        NepaleseRupee = 102,
        NetherlandsAntillianGuilder = 103,
        NewIsraeliSheqel = 104,
        NewLeu = 105,
        NewTaiwanDollar = 106,
        NewTurkishLira = 107,
        NewZealandDollar = 108,
        Ngultrum = 109,
        NorthKoreanWon = 110,
        NorwegianKrone = 111,
        NuevoSol = 112,
        Ouguiya = 113,
        Paanga = 114,
        PakistanRupee = 115,
        Pataca = 116,
        PesoUruguayo = 117,
        PhilippinePeso = 118,
        PoundSterling = 119,
        Pula = 120,
        QatariRial = 121,
        Quetzal = 122,
        Rand = 123,
        RialOmani = 124,
        Riel = 125,
        Rufiyaa = 126,
        Rupiah = 127,
        RussianRuble = 128,
        RwandaFranc = 129,
        SaintHelenaPound = 130,
        SaudiRiyal = 131,
        SerbianDinar = 132,
        SeychellesRupee = 133,
        SingaporeDollar = 134,
        SlovakKoruna = 135,
        SolomonIslandsDollar = 136,
        Som = 137,
        SomaliShilling = 138,
        Somoni = 139,
        SriLankaRupee = 140,
        SudanesePound = 141,
        SurinamDollar = 142,
        SwedishKrona = 143,
        SwissFranc = 144,
        SyrianPound = 145,
        Taka = 146,
        Tala = 147,
        TanzanianShilling = 148,
        Tenge = 149,
        TrinidadAndTobagoDollar = 150,
        Tugrik = 151,
        TunisianDinar = 152,
        UAEDirham = 153,
        USDollar = 154,
        UgandaShilling = 155,
        UnidadDeValorReal = 156,
        UnidadesDeFomento = 157,
        UruguayPesoEnUnidadesIndexadas = 158,
        UzbekistanSum = 159,
        Vatu = 160,
        Won = 161,
        YemeniRial = 162,
        Yen = 163,
        YuanRenminbi = 164,
        ZambiaKwacha = 165,
        ZimbabweDollar = 166,
        Zloty = 167,
        LastCurrency = Zloty
    };

    enum Continent
    {
        None = 0,
        NorthAmerica = 1,
        SouthAmerica = 2,
        Europe = 3,
        Africa = 4,
        Asia = 5,
        Australia = 6,
        Antarctica = 7,
        LastContinent = Antarctica
    };

    static QString countryToISO2Letter(QLocale::Country country);
    static QString countryToISO3Letter(QLocale::Country country);

    static Currency currencyForCountry(QLocale::Country country);
    static QString currencyToCode(Currency currency);
    static QString currencyToName(Currency currency);
    static QString currencyToSymbol(Currency currency);

    static QString continentToName(Continent continent);
    static Continent continentForCountry(QLocale::Country country);
};

#endif // QXTLOCALE_H
