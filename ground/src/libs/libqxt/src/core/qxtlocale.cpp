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
#include "qxtlocale.h"
#include "qxtlocale_data_p.h"
#include <QCoreApplication>
#include <QDebug>

/*!
    \class QxtLocale
    \inmodule QxtCore
    \brief The QxtLocale class has some additional data and functionality, missing in QLocale.

    QxtLocale defines currencies and continents. Methods are available to
    gather various mappings and information.
 */

/*!
    \enum QxtLocale::Continent

    This enumerated type is used to specify a language.
*/

/*!
    \enum QxtLocale::Currency

    This enumerated type is used to specify a currency.
*/

/*!
    \fn QxtLocale::currencyToCode(Currency currency)

    This static method returns the 3-letter code for the given \a currency.
 */
QString QxtLocale::currencyToCode(Currency currency)
{
    QString code;
    code.resize(3);
    const unsigned char *c = currency_code_list + 3 * (uint(currency));
    code[0] = ushort(c[0]);
    code[1] = ushort(c[1]);
    code[2] = ushort(c[2]);
    return code;
}

/*!
    \fn QxtLocale::countryToISO2Letter(QLocale::Country country)

    This static method returns the 2-letter ISO 3166 code for the given \a country.
 */
QString QxtLocale::countryToISO2Letter(QLocale::Country country)
{
    if (country == QLocale::AnyCountry)
        return QString();

    QString code;
    code.resize(2);
    const unsigned char *c = two_letter_country_code_list + 2 * (uint(country));
    code[0] = ushort(c[0]);
    code[1] = ushort(c[1]);
    return code;
}

/*!
    \fn QxtLocale::countryToISO3Letter(QLocale::Country country)

    This static method returns the 3-letter ISO 3166 code for the given \a country.
 */
QString QxtLocale::countryToISO3Letter(QLocale::Country country)
{
    if (country == QLocale::AnyCountry)
        return QString();

    QString code;
    code.resize(3);
    const unsigned char *c = three_letter_country_code_list + 3 * (uint(country));
    code[0] = ushort(c[0]);
    code[1] = ushort(c[1]);
    code[2] = ushort(c[2]);
    return code;
}

/*!
    \fn QxtLocale::currencyToName(Currency currency)

    This static method returns the translated name for given \a currency.
 */
QString QxtLocale::currencyToName(Currency currency)
{
    return QCoreApplication::instance()->translate("QxtLocale", currency_names[currency]);
}

/*!
    \fn QxtLocale::currencyForCountry(QLocale::Country country)

    This static method returns the currency for the given \a country.
 */
QxtLocale::Currency QxtLocale::currencyForCountry(QLocale::Country country)
{
    return currency_for_country_list[country];
}

/*!
    \fn QxtLocale::currencyToSymbol(Currency currency)

    This static method returns the symbol for the given \a currency.
 */
QString QxtLocale::currencyToSymbol(Currency currency)
{
    QChar* data = symbol_for_country_list[currency];
    int size = data[0].cell();
    return QString(data + 1, size);
}

/*!
    \fn QxtLocale::continentToName(Continent continent)

    This static method returns the translated name for the given \a continent.
 */
QString QxtLocale::continentToName(Continent continent)
{
    return QCoreApplication::instance()->translate("QxtLocale", continent_names[continent]);
}

/*!
    This static method returns the continent for the given \a country.
 */
QxtLocale::Continent QxtLocale::continentForCountry(QLocale::Country country)
{
    return continent_for_country_list[country];
}
