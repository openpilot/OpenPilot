#include <QDebug>

//#include <stdlib.h>
//#include <string.h>
//#include <locale.h>
#include <glib/gprintf.h>
//#include <unistd.h>
//#include <libintl.h>

#include <gst/gst.h>
//#include "gst/gst-i18n-app.h"

#include "gst_global.h"

#include "gst-plugins-bad/sys/winscreencap/gstwinscreencap.h"

//#include <gst/controller/gstcontroller.h>

//#include "tools.h"

#define ngettext(MSG, MSG_PLURAL, COUNT) ((COUNT <= 1) ? MSG : MSG_PLURAL)

static bool initialized = false;

static void print_element_list(gboolean print_all);
static int print_element_info(GstElementFactory *factory, gboolean print_names);

// Not thread safe. Does it need to be?
void gst::init(int *argc, char **argv[])
{
    if (initialized)
        return;
    initialized = true;

    qDebug() << "gstreamer - initializing";
    gst_init(argc, argv);

    qDebug() << QString("gstreamer - version %0").arg(gst_version_string());

    qDebug() << "gstreamer - registering plugins";
    if (!register_plugin2()) {
        qDebug() << "gstreamer - failed to register plugin";
    }

//    print_element_list(false);
}

QString gst::version(void)
{
    init(NULL, NULL);
    return QString(gst_version_string());
}

QList<QString> gst::pluginList()
{
    init(NULL, NULL);

    QList<QString> pluginList;

    GList *plugins, *orig_plugins;

    orig_plugins = plugins = gst_default_registry_get_plugin_list();
    while (plugins) {
        GstPlugin *plugin;

        plugin = (GstPlugin *) (plugins->data);
        plugins = g_list_next(plugins);

        if (plugin->flags & GST_PLUGIN_FLAG_BLACKLISTED) {
            continue;
        }

        pluginList << QString(plugin->desc.name);
    }

    gst_plugin_list_free(orig_plugins);

    return pluginList;
}

QList<QString> gst::elementList(QString pluginName)
{
    init(NULL, NULL);

    QList<QString> elementList;

    //int plugincount = 0, featurecount = 0, blacklistcount = 0;
    GList *plugins, *orig_plugins;

    orig_plugins = plugins = gst_default_registry_get_plugin_list();
    while (plugins) {
        GList *features, *orig_features;
        GstPlugin *plugin;

        plugin = (GstPlugin *) (plugins->data);
        plugins = g_list_next(plugins);
        //plugincount++;

        if (plugin->flags & GST_PLUGIN_FLAG_BLACKLISTED) {
            //blacklistcount++;
            continue;
        }

        orig_features = features = gst_registry_get_feature_list_by_plugin(gst_registry_get_default(),
                plugin->desc.name);
        while (features) {
            GstPluginFeature *feature;

            if (G_UNLIKELY(features->data == NULL))
                goto next;
            feature = GST_PLUGIN_FEATURE(features->data);
            //featurecount++;

            if (GST_IS_ELEMENT_FACTORY(feature)) {
                GstElementFactory *factory;

                factory = GST_ELEMENT_FACTORY(feature);
                elementList << QString(GST_PLUGIN_FEATURE_NAME(factory));
//                if (print_all)
//                    print_element_info(factory, TRUE);
//                else
//                    g_print("%s:  %s: %s\n", plugin->desc.name, GST_PLUGIN_FEATURE_NAME(factory),
//                            gst_element_factory_get_longname(factory));
            } else if (GST_IS_INDEX_FACTORY(feature)) {
                GstIndexFactory *factory;

                factory = GST_INDEX_FACTORY(feature);
                elementList << QString(GST_PLUGIN_FEATURE_NAME(factory));
//                if (!print_all)
//                    g_print("%s:  %s: %s\n", plugin->desc.name, GST_PLUGIN_FEATURE_NAME(factory), factory->longdesc);
            } else if (GST_IS_TYPE_FIND_FACTORY(feature)) {
                GstTypeFindFactory *factory;

                factory = GST_TYPE_FIND_FACTORY(feature);
                elementList << QString(gst_plugin_feature_get_name(feature));

//                if (!print_all)
//                    g_print("%s: %s: ", plugin->desc.name, gst_plugin_feature_get_name(feature));
//                if (factory->extensions) {
//                    guint i = 0;
//
//                    while (factory->extensions[i]) {
//                        if (!print_all)
//                            g_print("%s%s", i > 0 ? ", " : "", factory->extensions[i]);
//                        i++;
//                    }
//                    if (!print_all)
//                        g_print("\n");
//                } else {
//                    if (!print_all)
//                        g_print("no extensions\n");
//                }
            } else {
//                if (!print_all)
//                    n_print("%s:  %s (%s)\n", plugin->desc.name, GST_PLUGIN_FEATURE_NAME(feature),
//                            g_type_name(G_OBJECT_TYPE(feature)));
            }

            next: features = g_list_next(features);
        }

        gst_plugin_feature_list_free(orig_features);
    }

    gst_plugin_list_free(orig_plugins);

    return elementList;
}

static char *_name = NULL;

static void n_print(const char *format, ...)
{
    va_list args;

    if (_name)
        g_print("%s", _name);

    va_start(args, format);
    g_vprintf(format, args);
    va_end(args);
}

static gboolean print_field(GQuark field, const GValue * value, gpointer pfx)
{
    gchar *str = gst_value_serialize(value);

    n_print("%s  %15s: %s\n", (gchar *) pfx, g_quark_to_string(field), str);
    g_free(str);
    return TRUE;
}

static void print_caps(const GstCaps * caps, const gchar * pfx)
{
    guint i;

    g_return_if_fail(caps != NULL);

    if (gst_caps_is_any(caps)) {
        n_print("%sANY\n", pfx);
        return;
    }
    if (gst_caps_is_empty(caps)) {
        n_print("%sEMPTY\n", pfx);
        return;
    }

    for (i = 0; i < gst_caps_get_size(caps); i++) {
        GstStructure *structure = gst_caps_get_structure(caps, i);

        n_print("%s%s\n", pfx, gst_structure_get_name(structure));
        gst_structure_foreach(structure, print_field, (gpointer) pfx);
    }
}

#if 0
static void
print_formats (const GstFormat * formats)
{
    while (formats && *formats) {
        const GstFormatDefinition *definition;

        definition = gst_format_get_details (*formats);
        if (definition)
        n_print ("\t\t(%d):\t%s (%s)\n", *formats,
                definition->nick, definition->description);
        else
        n_print ("\t\t(%d):\tUnknown format\n", *formats);

        formats++;
    }
}
#endif

static void print_query_types(const GstQueryType * types)
{
    while (types && *types) {
        const GstQueryTypeDefinition *definition;

        definition = gst_query_type_get_details(*types);
        if (definition)
            n_print("\t\t(%d):\t%s (%s)\n", *types, definition->nick, definition->description);
        else
            n_print("\t\t(%d):\tUnknown query format\n", *types);

        types++;
    }
}

#if 0
static void
print_event_masks (const GstEventMask * masks)
{
    GType event_type;
    GEnumClass *klass;
    GType event_flags;
    GFlagsClass *flags_class = NULL;

    event_type = gst_event_type_get_type ();
    klass = (GEnumClass *) g_type_class_ref (event_type);

    while (masks && masks->type) {
        GEnumValue *value;
        gint flags = 0, index = 0;

        switch (masks->type) {
            case GST_EVENT_SEEK:
            flags = masks->flags;
            event_flags = gst_seek_type_get_type ();
            flags_class = (GFlagsClass *) g_type_class_ref (event_flags);
            break;
            default:
            break;
        }

        value = g_enum_get_value (klass, masks->type);
        g_print ("\t\t%s ", value->value_nick);

        while (flags) {
            GFlagsValue *value;

            if (flags & 1) {
                value = g_flags_get_first_value (flags_class, 1 << index);

                if (value)
                g_print ("| %s ", value->value_nick);
                else
                g_print ("| ? ");
            }
            flags >>= 1;
            index++;
        }
        g_print ("\n");

        masks++;
    }
}
#endif

static const char *
get_rank_name(char *s, gint rank)
{
    static const int ranks[4] = { GST_RANK_NONE, GST_RANK_MARGINAL, GST_RANK_SECONDARY, GST_RANK_PRIMARY };
    static const char *rank_names[4] = { "none", "marginal", "secondary", "primary" };
    int i;
    int best_i;

    best_i = 0;
    for (i = 0; i < 4; i++) {
        if (rank == ranks[i])
            return rank_names[i];
        if (abs(rank - ranks[i]) < abs(rank - ranks[best_i])) {
            best_i = i;
        }
    }

    sprintf(s, "%s %c %d", rank_names[best_i], (rank - ranks[best_i] > 0) ? '+' : '-', abs(ranks[best_i] - rank));

    return s;
}

static gboolean print_factory_details_meta_data(GQuark field_id, const GValue * value, gpointer user_data)
{
    gchar *val = g_strdup_value_contents(value);
    gchar *key = g_strdup(g_quark_to_string(field_id));

    key[0] = g_ascii_toupper(key[0]);
    n_print("  %s:\t\t%s\n", key, val);
    g_free(val);
    g_free(key);
    return TRUE;
}

static void print_factory_details_info(GstElementFactory * factory)
{
    char s[20];

    n_print("Factory Details:\n");
    n_print("  Long name:\t%s\n", factory->details.longname);
    n_print("  Class:\t%s\n", factory->details.klass);
    n_print("  Description:\t%s\n", factory->details.description);
    n_print("  Author(s):\t%s\n", factory->details.author);
    n_print("  Rank:\t\t%s (%d)\n", get_rank_name(s, GST_PLUGIN_FEATURE(factory)->rank),
            GST_PLUGIN_FEATURE(factory)->rank);
    if (factory->meta_data != NULL) {
        gst_structure_foreach((GstStructure *) factory->meta_data, print_factory_details_meta_data, NULL);
    }
    n_print("\n");
}

static void print_element_list(gboolean print_all)
{
    int plugincount = 0, featurecount = 0, blacklistcount = 0;
    GList *plugins, *orig_plugins;

    orig_plugins = plugins = gst_default_registry_get_plugin_list();
    while (plugins) {
        GList *features, *orig_features;
        GstPlugin *plugin;

        plugin = (GstPlugin *) (plugins->data);
        plugins = g_list_next(plugins);
        plugincount++;

        if (plugin->flags & GST_PLUGIN_FLAG_BLACKLISTED) {
            blacklistcount++;
            continue;
        }

        orig_features = features = gst_registry_get_feature_list_by_plugin(gst_registry_get_default(),
                plugin->desc.name);
        while (features) {
            GstPluginFeature *feature;

            if (G_UNLIKELY(features->data == NULL))
                goto next;
            feature = GST_PLUGIN_FEATURE(features->data);
            featurecount++;

            if (GST_IS_ELEMENT_FACTORY(feature)) {
                GstElementFactory *factory;

                factory = GST_ELEMENT_FACTORY(feature);
                if (print_all)
                    print_element_info(factory, TRUE);
                else
                    g_print("%s:  %s: %s\n", plugin->desc.name, GST_PLUGIN_FEATURE_NAME(factory),
                            gst_element_factory_get_longname(factory));
            } else if (GST_IS_INDEX_FACTORY(feature)) {
                GstIndexFactory *factory;

                factory = GST_INDEX_FACTORY(feature);
                if (!print_all)
                    g_print("%s:  %s: %s\n", plugin->desc.name, GST_PLUGIN_FEATURE_NAME(factory), factory->longdesc);
            } else if (GST_IS_TYPE_FIND_FACTORY(feature)) {
                GstTypeFindFactory *factory;

                factory = GST_TYPE_FIND_FACTORY(feature);
                if (!print_all)
                    g_print("%s: %s: ", plugin->desc.name, gst_plugin_feature_get_name(feature));
                if (factory->extensions) {
                    guint i = 0;

                    while (factory->extensions[i]) {
                        if (!print_all)
                            g_print("%s%s", i > 0 ? ", " : "", factory->extensions[i]);
                        i++;
                    }
                    if (!print_all)
                        g_print("\n");
                } else {
                    if (!print_all)
                        g_print("no extensions\n");
                }
            } else {
                if (!print_all)
                    n_print("%s:  %s (%s)\n", plugin->desc.name, GST_PLUGIN_FEATURE_NAME(feature),
                            g_type_name(G_OBJECT_TYPE(feature)));
            }

            next: features = g_list_next(features);
        }

        gst_plugin_feature_list_free(orig_features);
    }

    gst_plugin_list_free(orig_plugins);

    g_print("\n");
    g_print("Total count: ");
    g_print(ngettext("%d plugin", "%d plugins", plugincount), plugincount);
    if (blacklistcount) {
        g_print(" (");
        g_print(ngettext("%d blacklist entry", "%d blacklist entries", blacklistcount), blacklistcount);
        g_print(" not shown)");
    }
    g_print(", ");
    g_print(ngettext("%d feature", "%d features", featurecount), featurecount);
    g_print("\n");
}

static void print_plugin_info(GstPlugin * plugin)
{
    n_print("Plugin Details:\n");
    n_print("  Name:\t\t\t%s\n", plugin->desc.name);
    n_print("  Description:\t\t%s\n", plugin->desc.description);
    n_print("  Filename:\t\t%s\n", plugin->filename ? plugin->filename : "(null)");
    n_print("  Version:\t\t%s\n", plugin->desc.version);
    n_print("  License:\t\t%s\n", plugin->desc.license);
    n_print("  Source module:\t%s\n", plugin->desc.source);
    if (plugin->desc.release_datetime != NULL) {
        const gchar *tz = "(UTC)";
        gchar *str, *sep;

        /* may be: YYYY-MM-DD or YYYY-MM-DDTHH:MMZ */
        /* YYYY-MM-DDTHH:MMZ => YYYY-MM-DD HH:MM (UTC) */
        str = g_strdup(plugin->desc.release_datetime);
        sep = strstr(str, "T");
        if (sep != NULL) {
            *sep = ' ';
            sep = strstr(sep + 1, "Z");
            if (sep != NULL)
                *sep = ' ';
        } else {
            tz = "";
        }
        n_print("  Source release date:\t%s%s\n", str, tz);
        g_free(str);
    }
    n_print("  Binary package:\t%s\n", plugin->desc.package);
    n_print("  Origin URL:\t\t%s\n", plugin->desc.origin);
    n_print("\n");
}

static gchar *
flags_to_string(GFlagsValue * vals, guint flags)
{
    GString *s = NULL;
    guint flags_left, i;

    /* first look for an exact match and count the number of values */
    for (i = 0; vals[i].value_name != NULL; ++i) {
        if (vals[i].value == flags)
            return g_strdup(vals[i].value_nick);
    }

    s = g_string_new(NULL);

    /* we assume the values are sorted from lowest to highest value */
    flags_left = flags;
    while (i > 0) {
        --i;
        if (vals[i].value != 0 && (flags_left & vals[i].value) == vals[i].value) {
            if (s->len > 0)
                g_string_append(s, " | ");
            g_string_append(s, vals[i].value_nick);
            flags_left -= vals[i].value;
            if (flags_left == 0)
                break;
        }
    }

    if (s->len == 0)
        g_string_assign(s, "(none)");

    return g_string_free(s, FALSE);
}

#define KNOWN_PARAM_FLAGS \
  (G_PARAM_CONSTRUCT | G_PARAM_CONSTRUCT_ONLY | \
  G_PARAM_LAX_VALIDATION |  G_PARAM_STATIC_STRINGS | \
  G_PARAM_READABLE | G_PARAM_WRITABLE | GST_PARAM_CONTROLLABLE | \
  GST_PARAM_MUTABLE_PLAYING | GST_PARAM_MUTABLE_PAUSED | \
  GST_PARAM_MUTABLE_READY)

static void print_element_properties_info(GstElement * element)
{
    GParamSpec **property_specs;
    guint num_properties, i;
    gboolean readable;
    gboolean first_flag;

    property_specs = g_object_class_list_properties(G_OBJECT_GET_CLASS(element), &num_properties);
    n_print("\n");
    n_print("Element Properties:\n");

    for (i = 0; i < num_properties; i++) {
        GValue value = { 0, };
        GParamSpec *param = property_specs[i];

        readable = FALSE;

        g_value_init(&value, param->value_type);

        n_print("  %-20s: %s\n", g_param_spec_get_name(param), g_param_spec_get_blurb(param));

        first_flag = TRUE;
        n_print("%-23.23s flags: ", "");
        if (param->flags & G_PARAM_READABLE) {
            g_object_get_property(G_OBJECT(element), param->name, &value);
            readable = TRUE;
            g_print("%s%s", (first_flag) ? "" : ", ", "readable");
            first_flag = FALSE;
        }
        if (param->flags & G_PARAM_WRITABLE) {
            g_print("%s%s", (first_flag) ? "" : ", ", "writable");
            first_flag = FALSE;
        }
        if (param->flags & GST_PARAM_CONTROLLABLE) {
            g_print(", %s", "controllable");
            first_flag = FALSE;
        }
        if (param->flags & GST_PARAM_MUTABLE_PLAYING) {
            g_print(", %s", "changeable in NULL, READY, PAUSED or PLAYING state");
        } else if (param->flags & GST_PARAM_MUTABLE_PAUSED) {
            g_print(", %s", "changeable only in NULL, READY or PAUSED state");
        } else if (param->flags & GST_PARAM_MUTABLE_READY) {
            g_print(", %s", "changeable only in NULL or READY state");
        }
        if (param->flags & ~KNOWN_PARAM_FLAGS) {
            g_print("%s0x%0x", (first_flag) ? "" : ", ", param->flags & ~KNOWN_PARAM_FLAGS);
        }
        n_print("\n");

        switch (G_VALUE_TYPE(&value)) {
        case G_TYPE_STRING: {
            GParamSpecString *pstring = G_PARAM_SPEC_STRING(param);

            n_print("%-23.23s String. ", "");

            if (pstring->default_value == NULL)
                g_print("Default: null ");
            else
                g_print("Default: \"%s\" ", pstring->default_value);

            if (readable) {
                const char *string_val = g_value_get_string(&value);

                if (string_val == NULL)
                    g_print("Current: null");
                else
                    g_print("Current: \"%s\"", string_val);
            }
            break;
        }
        case G_TYPE_BOOLEAN: {
            GParamSpecBoolean *pboolean = G_PARAM_SPEC_BOOLEAN(param);

            n_print("%-23.23s Boolean. ", "");
            g_print("Default: %s ", (pboolean->default_value ? "true" : "false"));
            if (readable)
                g_print("Current: %s", (g_value_get_boolean(&value) ? "true" : "false"));
            break;
        }
        case G_TYPE_ULONG: {
            GParamSpecULong *pulong = G_PARAM_SPEC_ULONG(param);

            n_print("%-23.23s Unsigned Long. ", "");
            g_print("Range: %lu - %lu Default: %lu ", pulong->minimum, pulong->maximum, pulong->default_value);
            if (readable)
                g_print("Current: %lu", g_value_get_ulong(&value));
            break;
        }
        case G_TYPE_LONG: {
            GParamSpecLong *plong = G_PARAM_SPEC_LONG(param);

            n_print("%-23.23s Long. ", "");
            g_print("Range: %ld - %ld Default: %ld ", plong->minimum, plong->maximum, plong->default_value);
            if (readable)
                g_print("Current: %ld", g_value_get_long(&value));
            break;
        }
        case G_TYPE_UINT: {
            GParamSpecUInt *puint = G_PARAM_SPEC_UINT(param);

            n_print("%-23.23s Unsigned Integer. ", "");
            g_print("Range: %u - %u Default: %u ", puint->minimum, puint->maximum, puint->default_value);
            if (readable)
                g_print("Current: %u", g_value_get_uint(&value));
            break;
        }
        case G_TYPE_INT: {
            GParamSpecInt *pint = G_PARAM_SPEC_INT(param);

            n_print("%-23.23s Integer. ", "");
            g_print("Range: %d - %d Default: %d ", pint->minimum, pint->maximum, pint->default_value);
            if (readable)
                g_print("Current: %d", g_value_get_int(&value));
            break;
        }
        case G_TYPE_UINT64: {
            GParamSpecUInt64 *puint64 = G_PARAM_SPEC_UINT64(param);

            n_print("%-23.23s Unsigned Integer64. ", "");
            g_print ("Range: %" G_GUINT64_FORMAT " - %" G_GUINT64_FORMAT
                    " Default: %" G_GUINT64_FORMAT " ",
                    puint64->minimum, puint64->maximum, puint64->default_value);
            if (readable)
            g_print ("Current: %" G_GUINT64_FORMAT, g_value_get_uint64 (&value));
            break;
        }
        case G_TYPE_INT64: {
            GParamSpecInt64 *pint64 = G_PARAM_SPEC_INT64(param);

            n_print("%-23.23s Integer64. ", "");
            g_print ("Range: %" G_GINT64_FORMAT " - %" G_GINT64_FORMAT
                    " Default: %" G_GINT64_FORMAT " ",
                    pint64->minimum, pint64->maximum, pint64->default_value);
            if (readable)
            g_print ("Current: %" G_GINT64_FORMAT, g_value_get_int64 (&value));
            break;
        }
        case G_TYPE_FLOAT: {
            GParamSpecFloat *pfloat = G_PARAM_SPEC_FLOAT(param);

            n_print("%-23.23s Float. ", "");
            g_print("Range: %15.7g - %15.7g Default: %15.7g ", pfloat->minimum, pfloat->maximum, pfloat->default_value);
            if (readable)
                g_print("Current: %15.7g", g_value_get_float(&value));
            break;
        }
        case G_TYPE_DOUBLE: {
            GParamSpecDouble *pdouble = G_PARAM_SPEC_DOUBLE(param);

            n_print("%-23.23s Double. ", "");
            g_print("Range: %15.7g - %15.7g Default: %15.7g ", pdouble->minimum, pdouble->maximum,
                    pdouble->default_value);
            if (readable)
                g_print("Current: %15.7g", g_value_get_double(&value));
            break;
        }
        default:
            if (param->value_type == GST_TYPE_CAPS) {
                const GstCaps *caps = gst_value_get_caps(&value);

                if (!caps)
                    n_print("%-23.23s Caps (NULL)", "");
                else {
                    print_caps(caps, "                           ");
                }
            } else if (G_IS_PARAM_SPEC_ENUM(param)) {
                GParamSpecEnum *penum = G_PARAM_SPEC_ENUM(param);
                GEnumValue *values;
                guint j = 0;
                gint enum_value;
                const gchar *def_val_nick = "", *cur_val_nick = "";

                values = G_ENUM_CLASS(g_type_class_ref(param->value_type))->values;
                enum_value = g_value_get_enum(&value);

                while (values[j].value_name) {
                    if (values[j].value == enum_value)
                        cur_val_nick = values[j].value_nick;
                    if (values[j].value == penum->default_value)
                        def_val_nick = values[j].value_nick;
                    j++;
                }

                n_print("%-23.23s Enum \"%s\" Default: %d, \"%s\" Current: %d, \"%s\"", "",
                        g_type_name(G_VALUE_TYPE(&value)), penum->default_value, def_val_nick, enum_value,
                        cur_val_nick);

                j = 0;
                while (values[j].value_name) {
                    g_print("\n");
                    if (_name)
                        g_print("%s", _name);
                    g_print("%-23.23s    (%d): %-16s - %s", "", values[j].value, values[j].value_nick,
                            values[j].value_name);
                    j++;
                }
                /* g_type_class_unref (ec); */
            } else if (G_IS_PARAM_SPEC_FLAGS(param)) {
                GParamSpecFlags *pflags = G_PARAM_SPEC_FLAGS(param);
                GFlagsValue *vals;
                gchar *cur, *def;

                vals = pflags->flags_class->values;

                cur = flags_to_string(vals, g_value_get_flags(&value));
                def = flags_to_string(vals, pflags->default_value);

                n_print("%-23.23s Flags \"%s\" Default: 0x%08x, \"%s\" Current: 0x%08x, \"%s\"", "",
                        g_type_name(G_VALUE_TYPE(&value)), pflags->default_value, def, g_value_get_flags(&value), cur);

                while (vals[0].value_name) {
                    g_print("\n");
                    if (_name)
                        g_print("%s", _name);
                    g_print("%-23.23s    (0x%08x): %-16s - %s", "", vals[0].value, vals[0].value_nick,
                            vals[0].value_name);
                    ++vals;
                }

                g_free(cur);
                g_free(def);
            } else if (G_IS_PARAM_SPEC_OBJECT(param)) {
                n_print("%-23.23s Object of type \"%s\"", "", g_type_name(param->value_type));
            } else if (G_IS_PARAM_SPEC_BOXED(param)) {
                n_print("%-23.23s Boxed pointer of type \"%s\"", "", g_type_name(param->value_type));
            } else if (G_IS_PARAM_SPEC_POINTER(param)) {
                if (param->value_type != G_TYPE_POINTER) {
                    n_print("%-23.23s Pointer of type \"%s\".", "", g_type_name(param->value_type));
                } else {
                    n_print("%-23.23s Pointer.", "");
                }
            } else if (param->value_type == G_TYPE_VALUE_ARRAY) {
                GParamSpecValueArray *pvarray = G_PARAM_SPEC_VALUE_ARRAY(param);

                if (pvarray->element_spec) {
                    n_print("%-23.23s Array of GValues of type \"%s\"", "",
                            g_type_name(pvarray->element_spec->value_type));
                } else {
                    n_print("%-23.23s Array of GValues", "");
                }
            } else if (GST_IS_PARAM_SPEC_FRACTION(param)) {
                GstParamSpecFraction *pfraction = GST_PARAM_SPEC_FRACTION(param);

                n_print("%-23.23s Fraction. ", "");

                g_print("Range: %d/%d - %d/%d Default: %d/%d ", pfraction->min_num, pfraction->min_den,
                        pfraction->max_num, pfraction->max_den, pfraction->def_num, pfraction->def_den);
                if (readable)
                    g_print("Current: %d/%d", gst_value_get_fraction_numerator(&value),
                            gst_value_get_fraction_denominator(&value));

            } else if (GST_IS_PARAM_SPEC_MINI_OBJECT(param)) {
                n_print("%-23.23s MiniObject of type \"%s\"", "", g_type_name(param->value_type));
            } else {
                n_print("%-23.23s Unknown type %ld \"%s\"", "", param->value_type, g_type_name(param->value_type));
            }
            break;
        }
        if (!readable)
            g_print(" Write only\n");
        else
            g_print("\n");

        g_value_reset(&value);
    }
    if (num_properties == 0)
        n_print("  none\n");

    g_free(property_specs);
}

static int print_element_info(GstElementFactory *factory, gboolean print_names)
{
    GstElement *element;
//    gint maxlevel = 0;

    factory = GST_ELEMENT_FACTORY(gst_plugin_feature_load(GST_PLUGIN_FEATURE(factory)));

    if (!factory) {
        g_print("element plugin couldn't be loaded\n");
        return -1;
    }

    element = gst_element_factory_create(factory, NULL);
    if (!element) {
        g_print("couldn't construct element for some reason\n");
        return -1;
    }

    if (print_names)
        _name = g_strdup_printf("%s: ", GST_PLUGIN_FEATURE(factory)->name);
    else
        _name = NULL;

    print_factory_details_info(factory);
    if (GST_PLUGIN_FEATURE(factory)->plugin_name) {
        GstPlugin *plugin;

        plugin = gst_registry_find_plugin(gst_registry_get_default(), GST_PLUGIN_FEATURE(factory)->plugin_name);
        if (plugin) {
            print_plugin_info(plugin);
        }
    }

//    print_hierarchy(G_OBJECT_TYPE(element), 0, &maxlevel);
//    print_interfaces(G_OBJECT_TYPE(element));
//
//    print_pad_templates_info(element, factory);
//    print_element_flag_info(element);
//    print_implementation_info(element);
//    print_clocking_info(element);
//    print_index_info(element);
//    print_uri_handler_info(element);
//    print_pad_info(element);
    print_element_properties_info(element);
//    print_signal_info(element);
//    print_children_info(element);

    gst_object_unref(element);
    gst_object_unref(factory);
    g_free(_name);

    return 0;
}
