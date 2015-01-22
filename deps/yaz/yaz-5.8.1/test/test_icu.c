/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/* DO NOT EDIT THIS FILE IF YOUR EDITOR DOES NOT SUPPORT UTF-8 */


#if HAVE_CONFIG_H
#include "config.h"
#endif

#define USE_TIMING 0
#if USE_TIMING
#include <yaz/timing.h>
#endif

#include <yaz/test.h>
#include <yaz/log.h>
#include <yaz/wrbuf.h>

#if YAZ_HAVE_ICU
#include <yaz/icu_I18N.h>
#include <unicode/uclean.h>

#if YAZ_POSIX_THREADS
#include <pthread.h>
#endif

#if YAZ_HAVE_XML2
#include <libxml/xmlmemory.h>
#endif

#include <string.h>
#include <stdlib.h>

#define MAX_KEY_SIZE 256
struct icu_termmap
{
    uint8_t sort_key[MAX_KEY_SIZE]; /* standard C string '\0' terminated */
    char disp_term[MAX_KEY_SIZE];  /* standard C utf-8 string */
};


static int icu_termmap_cmp(const void *vp1, const void *vp2)
{
    struct icu_termmap *itmp1 = *(struct icu_termmap **) vp1;
    struct icu_termmap *itmp2 = *(struct icu_termmap **) vp2;

    int cmp = 0;

    cmp = strcmp((const char *)itmp1->sort_key,
                 (const char *)itmp2->sort_key);
    return cmp;
}


static int test_icu_casemap(const char *locale, char action,
                            const char *src8cstr, const char *chk8cstr)
{
    int success = 0;
    UErrorCode status = U_ZERO_ERROR;

    struct icu_buf_utf8 *src8 = icu_buf_utf8_create(0);
    struct icu_buf_utf8 *dest8 = icu_buf_utf8_create(0);
    struct icu_buf_utf16 *src16 = icu_buf_utf16_create(0);
    struct icu_buf_utf16 *dest16 = icu_buf_utf16_create(0);


    int src8cstr_len = strlen(src8cstr);
    int chk8cstr_len = strlen(chk8cstr);

    /* converting to UTF16 */
    icu_utf16_from_utf8_cstr(src16, src8cstr, &status);

    /* perform case mapping */
    icu_utf16_casemap(dest16, src16, locale, action, &status);

    /* converting to UTF8 */
    icu_utf16_to_utf8(dest8, dest16, &status);

    /* determine success */
    if (dest8->utf8
        && (dest8->utf8_len == strlen(chk8cstr))
        && !strcmp(chk8cstr, (const char *) dest8->utf8))
        success = 1;
    else
        success = 0;

    /* report failures */
    if (!success)
    {
        yaz_log(YLOG_WARN, "test_icu_casemap failed");
        yaz_log(YLOG_LOG, "Original string:   '%s' (%d)",
                src8cstr, src8cstr_len);
        yaz_log(YLOG_LOG, "icu_casemap '%s:%c' '%s' (%d)",
                locale, action, dest8->utf8, dest8->utf8_len);
        yaz_log(YLOG_LOG, "expected string:   '%s' (%d)",
                chk8cstr, chk8cstr_len);
    }

    /* clean the buffers */
    icu_buf_utf8_destroy(src8);
    icu_buf_utf8_destroy(dest8);
    icu_buf_utf16_destroy(src16);
    icu_buf_utf16_destroy(dest16);

    return success;
}

static void check_icu_casemap(void)
{
    /* Locale 'en' */

    /* successful tests */
    YAZ_CHECK(test_icu_casemap("en", 'l',
                               "A ReD fOx hunTS sQUirriLs",
                               "a red fox hunts squirrils"));

    YAZ_CHECK(test_icu_casemap("en", 'u',
                               "A ReD fOx hunTS sQUirriLs",
                               "A RED FOX HUNTS SQUIRRILS"));

    YAZ_CHECK(test_icu_casemap("en", 'f',
                               "A ReD fOx hunTS sQUirriLs",
                               "a red fox hunts squirrils"));

    YAZ_CHECK(test_icu_casemap("en", 't',
                               "A ReD fOx hunTS sQUirriLs",
                               "A Red Fox Hunts Squirrils"));

    /* Locale 'da' */

    /* success expected */
    YAZ_CHECK(test_icu_casemap("da", 'l',
                               "åh ÆbLE, øs fLØde i Åen efter bLåBærGRødeN",
                               "åh æble, øs fløde i åen efter blåbærgrøden"));

    YAZ_CHECK(test_icu_casemap("da", 'u',
                               "åh ÆbLE, øs fLØde i Åen efter bLåBærGRødeN",
                               "ÅH ÆBLE, ØS FLØDE I ÅEN EFTER BLÅBÆRGRØDEN"));

    YAZ_CHECK(test_icu_casemap("da", 'f',
                               "åh ÆbLE, øs fLØde i Åen efter bLåBærGRødeN",
                               "åh æble, øs fløde i åen efter blåbærgrøden"));

    YAZ_CHECK(test_icu_casemap("da", 't',
                               "åh ÆbLE, øs fLØde i Åen efter bLåBærGRødeN",
                               "Åh Æble, Øs Fløde I Åen Efter Blåbærgrøden"));

    /* Locale 'de' */

    /* success expected */
    YAZ_CHECK(test_icu_casemap("de", 'l',
                               "zWÖlf ärgerliche Würste rollen ÜBer die StRAße",
                               "zwölf ärgerliche würste rollen über die straße"));

    YAZ_CHECK(test_icu_casemap("de", 'u',
                               "zWÖlf ärgerliche Würste rollen ÜBer die StRAße",
                               "ZWÖLF ÄRGERLICHE WÜRSTE ROLLEN ÜBER DIE STRASSE"));

    YAZ_CHECK(test_icu_casemap("de", 'f',
                               "zWÖlf ärgerliche Würste rollen ÜBer die StRAße",
                               "zwölf ärgerliche würste rollen über die strasse"));

    YAZ_CHECK(test_icu_casemap("de", 't',
                               "zWÖlf ärgerliche Würste rollen ÜBer die StRAße",
                               "Zwölf Ärgerliche Würste Rollen Über Die Straße"));

}

static int test_icu_sortmap(const char *locale, int src_list_len,
                            const char **src_list, const char **chk_list)
{
    int success = 1;

    UErrorCode status = U_ZERO_ERROR;

    struct icu_buf_utf8 *buf8 = icu_buf_utf8_create(0);
    struct icu_buf_utf16 *buf16 = icu_buf_utf16_create(0);

    int i;

    struct icu_termmap *list[src_list_len];

    UCollator *coll = ucol_open(locale, &status);
    icu_check_status(status);

    if (U_FAILURE(status))
        return 0;

    /* assigning display terms and sort keys using buf 8 and buf16 */
    for (i = 0; i < src_list_len; i++)
    {

        list[i] = (struct icu_termmap *) malloc(sizeof(struct icu_termmap));

        /* copy display term */
        strcpy(list[i]->disp_term, src_list[i]);

        /* transforming to UTF16 */
        icu_utf16_from_utf8_cstr(buf16, list[i]->disp_term, &status);
        icu_check_status(status);

        /* computing sortkeys */
        icu_sortkey8_from_utf16(coll, buf8, buf16, &status);
        icu_check_status(status);

        /* assigning sortkeys */
        memcpy(list[i]->sort_key, buf8->utf8, buf8->utf8_len);
    }

    /* do the sorting */
    qsort(list, src_list_len, sizeof(struct icu_termmap *), icu_termmap_cmp);

    /* checking correct sorting */
    for (i = 0; i < src_list_len; i++)
    {
        if (0 != strcmp(list[i]->disp_term, chk_list[i])){
            success = 0;
        }
    }

    if (!success)
    {
        yaz_log(YLOG_LOG, "ERROR");
        yaz_log(YLOG_LOG, "Input str:'%s':", locale);
        for (i = 0; i < src_list_len; i++) {
            yaz_log(YLOG_LOG, "  '%s'", list[i]->disp_term);
        }
        yaz_log(YLOG_LOG, "ICU sort: '%s':", locale);
        for (i = 0; i < src_list_len; i++) {
            yaz_log(YLOG_LOG, " '%s'", list[i]->disp_term);
        }
        yaz_log(YLOG_LOG, "Expected: '%s':", locale);
        for (i = 0; i < src_list_len; i++) {
            yaz_log(YLOG_LOG, " '%s'", chk_list[i]);
        }
    }

    for (i = 0; i < src_list_len; i++)
        free(list[i]);

    ucol_close(coll);

    icu_buf_utf8_destroy(buf8);
    icu_buf_utf16_destroy(buf16);

    return success;
}

static void check_icu_sortmap(void)
{
    /* successful tests */
    size_t en_1_len = 6;
    const char *en_1_src[6] = {"z", "K", "a", "A", "Z", "k"};
    const char *en_1_cck[6] = {"a", "A", "k", "K", "z", "Z"};
    YAZ_CHECK(test_icu_sortmap("en", en_1_len, en_1_src, en_1_cck));
    YAZ_CHECK(test_icu_sortmap("en_AU", en_1_len, en_1_src, en_1_cck));
    YAZ_CHECK(test_icu_sortmap("en_CA", en_1_len, en_1_src, en_1_cck));
    YAZ_CHECK(test_icu_sortmap("en_GB", en_1_len, en_1_src, en_1_cck));
    YAZ_CHECK(test_icu_sortmap("en_US", en_1_len, en_1_src, en_1_cck));

    /* successful tests */
    {
        size_t da_1_len = 6;
        const char *da_1_src[6] = {"z", "å", "o", "æ", "a", "ø"};
        const char *da_1_cck[6] = {"a", "o", "z", "æ", "ø", "å"};
        YAZ_CHECK(test_icu_sortmap("da", da_1_len, da_1_src, da_1_cck));
        YAZ_CHECK(test_icu_sortmap("da_DK", da_1_len, da_1_src, da_1_cck));
    }
    /* successful tests */
    {
        size_t de_1_len = 9;
        const char *de_1_src[9] = {"u", "ä", "o", "t", "s", "ß", "ü", "ö", "a"};
        const char *de_1_cck[9] = {"a","ä", "o", "ö", "s", "ß", "t", "u", "ü"};
        YAZ_CHECK(test_icu_sortmap("de", de_1_len, de_1_src, de_1_cck));
        YAZ_CHECK(test_icu_sortmap("de_AT", de_1_len, de_1_src, de_1_cck));
        YAZ_CHECK(test_icu_sortmap("de_DE", de_1_len, de_1_src, de_1_cck));
    }
}

static int test_icu_normalizer(const char *rules8cstr,
                               const char *src8cstr,
                               const char *chk8cstr)
{
    int success = 0;

    UErrorCode status = U_ZERO_ERROR;

    struct icu_buf_utf16 *src16 = icu_buf_utf16_create(0);
    struct icu_buf_utf16 *dest16 = icu_buf_utf16_create(0);
    struct icu_buf_utf8 *dest8 = icu_buf_utf8_create(0);
    struct icu_transform *transform
        = icu_transform_create(rules8cstr, 'f', 0, &status);
    icu_check_status(status);

    icu_utf16_from_utf8_cstr(src16, src8cstr, &status);
    icu_check_status(status);

    icu_transform_trans(transform, dest16, src16, &status);
    icu_check_status(status);

    icu_utf16_to_utf8(dest8, dest16, &status);
    icu_check_status(status);


    if (!strcmp((const char *) dest8->utf8,
               (const char *) chk8cstr))
        success = 1;
    else
    {
        success = 0;
        yaz_log(YLOG_LOG, "Normalization");
        yaz_log(YLOG_LOG, " Rules:      '%s'", rules8cstr);
        yaz_log(YLOG_LOG, " Input:      '%s'", src8cstr);
        yaz_log(YLOG_LOG, " Normalized: '%s'", dest8->utf8);
        yaz_log(YLOG_LOG, " Expected:   '%s'", chk8cstr);
    }

    icu_transform_destroy(transform);
    icu_buf_utf16_destroy(src16);
    icu_buf_utf16_destroy(dest16);
    icu_buf_utf8_destroy(dest8);

    return success;
}

static void check_icu_normalizer(void)
{
    YAZ_CHECK(test_icu_normalizer("[:Punctuation:] Any-Remove",
                                  "Don't shoot!",
                                  "Dont shoot"));

    YAZ_CHECK(test_icu_normalizer("[:Control:] Any-Remove",
                                  "Don't\n shoot!",
                                  "Don't shoot!"));

    YAZ_CHECK(test_icu_normalizer("[:Decimal_Number:] Any-Remove",
                                  "This is 4 you!",
                                  "This is  you!"));

    YAZ_CHECK(test_icu_normalizer("Lower; [:^Letter:] Remove",
                                  "Don't shoot!",
                                  "dontshoot"));

    YAZ_CHECK(test_icu_normalizer("[:^Number:] Remove",
                                  "Monday 15th of April",
                                  "15"));

    YAZ_CHECK(test_icu_normalizer("Lower;"
                                  "[[:WhiteSpace:][:Punctuation:]] Remove",
                                  " word4you? ",
                                  "word4you"));

    YAZ_CHECK(test_icu_normalizer("NFD; [:Nonspacing Mark:] Remove; NFC",
                                  "à côté de l'alcôve ovoïde",
                                  "a cote de l'alcove ovoide"));
}

static int test_icu_tokenizer(const char *locale, char action,
                              const char *src8cstr, int count)
{
    int success = 1;

    UErrorCode status = U_ZERO_ERROR;
    struct icu_buf_utf16 *src16 = icu_buf_utf16_create(0);
    struct icu_buf_utf16 *tkn16 = icu_buf_utf16_create(0);
    struct icu_buf_utf8 *tkn8 = icu_buf_utf8_create(0);
    struct icu_tokenizer *tokenizer = 0;
    size_t org_start, org_len;

    /* transforming to UTF16 */
    icu_utf16_from_utf8_cstr(src16, src8cstr, &status);
    icu_check_status(status);

    /* set up tokenizer */
    tokenizer = icu_tokenizer_create(locale, action, &status);
    icu_check_status(status);
    YAZ_CHECK(tokenizer);

    /* attach text buffer to tokenizer */
    icu_tokenizer_attach(tokenizer, src16, &status);
    icu_check_status(status);

    /* perform work on tokens */
    while (icu_tokenizer_next_token(tokenizer, tkn16, &status,
                                    &org_start, &org_len))
    {
        icu_check_status(status);

        /* converting to UTF8 */
        icu_utf16_to_utf8(tkn8, tkn16, &status);
    }

    if (count != icu_tokenizer_token_count(tokenizer))
    {
        success = 0;
        yaz_log(YLOG_LOG, "Tokenizer '%s:%c' Error:", locale, action);
        yaz_log(YLOG_LOG, " Input:  '%s'", src8cstr);
        yaz_log(YLOG_LOG, " Tokens: %d", icu_tokenizer_token_count(tokenizer));
        yaz_log(YLOG_LOG, " Expected: %d", count);
    }

    icu_tokenizer_destroy(tokenizer);
    icu_buf_utf16_destroy(src16);
    icu_buf_utf16_destroy(tkn16);
    icu_buf_utf8_destroy(tkn8);

    return success;
}

static void check_icu_tokenizer(void)
{
    const char *en_str
        = "O Romeo, Romeo! wherefore art thou Romeo?";

    YAZ_CHECK(test_icu_tokenizer("en", 's', en_str, 2));
    YAZ_CHECK(test_icu_tokenizer("en", 'l', en_str, 7));
    YAZ_CHECK(test_icu_tokenizer("en", 'w', en_str, 16));
    YAZ_CHECK(test_icu_tokenizer("en", 'c', en_str, 41));

    {
        const char *da_str
            = "Blåbærtærte. Denne kage stammer fra Finland. "
            "Den er med blåbær, men alle sommerens forskellige bær kan bruges.";

        YAZ_CHECK(test_icu_tokenizer("da", 's', da_str, 3));
        YAZ_CHECK(test_icu_tokenizer("dar", 'l', da_str, 17));
        YAZ_CHECK(test_icu_tokenizer("da", 'w', da_str, 37));
        YAZ_CHECK(test_icu_tokenizer("da", 'c', da_str, 110));
    }
}

static void check_icu_chain(void)
{
    const char *en_str
        = "O Romeo, Romeo! wherefore art thou\t Romeo?";

    UErrorCode status = U_ZERO_ERROR;
    struct icu_chain *chain = 0;

    const char *xml_str = "<icu locale=\"en\">"
        "<transform rule=\"[:Control:] Any-Remove\"/>"
        "<tokenize rule=\"l\"/>"
        "<transform rule=\"[[:WhiteSpace:][:Punctuation:]] Remove\"/>"
        "<display/>"
        "<casemap rule=\"l\"/>"
        "</icu>";


    xmlDoc *doc = xmlParseMemory(xml_str, strlen(xml_str));
    xmlNode *xml_node = xmlDocGetRootElement(doc);
    YAZ_CHECK(xml_node);

    chain = icu_chain_xml_config(xml_node, 0, &status);

    xmlFreeDoc(doc);
    YAZ_CHECK(chain);
    if (!chain)
        return;

    YAZ_CHECK(icu_chain_assign_cstr(chain, en_str, &status));

    while (icu_chain_next_token(chain, &status))
    {
        yaz_log(YLOG_LOG, "%d '%s' '%s'",
                icu_chain_token_number(chain),
                icu_chain_token_norm(chain),
                icu_chain_token_display(chain));
    }

    YAZ_CHECK_EQ(icu_chain_token_number(chain), 7);


    YAZ_CHECK(icu_chain_assign_cstr(chain, "what is this?", &status));

    while (icu_chain_next_token(chain, &status))
    {
        yaz_log(YLOG_LOG, "%d '%s' '%s'",
                icu_chain_token_number(chain),
                icu_chain_token_norm(chain),
                icu_chain_token_display(chain));
    }


    YAZ_CHECK_EQ(icu_chain_token_number(chain), 3);

    icu_chain_destroy(chain);
}


static void check_bug_1140(void)
{
    UErrorCode status = U_ZERO_ERROR;
    struct icu_chain *chain = 0;

    const char *xml_str = "<icu locale=\"en\">"

        /* if the first rule is normalize instead. Then it works */
#if 0
        "<transform rule=\"[:Control:] Any-Remove\"/>"
#endif
        "<tokenize rule=\"l\"/>"
        "<transform rule=\"[[:WhiteSpace:][:Punctuation:]] Remove\"/>"
        "<display/>"
        "<casemap rule=\"l\"/>"
        "</icu>";


    xmlDoc *doc = xmlParseMemory(xml_str, strlen(xml_str));
    xmlNode *xml_node = xmlDocGetRootElement(doc);
    YAZ_CHECK(xml_node);

    chain = icu_chain_xml_config(xml_node, 0, &status);

    xmlFreeDoc(doc);
    YAZ_CHECK(chain);
    if (!chain)
        return;

    YAZ_CHECK(icu_chain_assign_cstr(
                  chain,  "O Romeo, Romeo! wherefore art thou\t Romeo?",
                  &status));

    while (icu_chain_next_token(chain, &status))
    {
        ;
        /* printf("%d '%s' '%s'\n",
           icu_chain_token_number(chain),
           icu_chain_token_norm(chain),
           icu_chain_token_display(chain)); */
    }


    YAZ_CHECK_EQ(icu_chain_token_number(chain), 7);

    YAZ_CHECK(icu_chain_assign_cstr(chain, "what is this?", &status));

    while (icu_chain_next_token(chain, &status))
    {
        ;
        /* printf("%d '%s' '%s'\n",
           icu_chain_token_number(chain),
           icu_chain_token_norm(chain),
           icu_chain_token_display(chain)); */
    }

    /* we expect 'what' 'is' 'this', i.e. 3 tokens */
    YAZ_CHECK_EQ(icu_chain_token_number(chain), 3);

    icu_chain_destroy(chain);
}


static void check_chain_empty_token(void)
{
    UErrorCode status = U_ZERO_ERROR;
    struct icu_chain *chain = 0;

    const char *xml_str = "<icu locale=\"en\">"
        "<tokenize rule=\"w\"/>"
        "<transform rule=\"[[:WhiteSpace:][:Punctuation:]] Remove\"/>"
        "</icu>";

    xmlDoc *doc = xmlParseMemory(xml_str, strlen(xml_str));
    xmlNode *xml_node = xmlDocGetRootElement(doc);
    YAZ_CHECK(xml_node);

    chain = icu_chain_xml_config(xml_node, 0, &status);

    xmlFreeDoc(doc);
    YAZ_CHECK(chain);

    YAZ_CHECK(icu_chain_assign_cstr(
                  chain,  "a string with 15 tokenss and 8 displays",
                  &status));

    while (icu_chain_next_token(chain, &status))
    {
        ;
        /* printf("%d '%s' '%s'\n",
           icu_chain_token_number(chain),
           icu_chain_token_norm(chain),
           icu_chain_token_display(chain)); */
    }

    YAZ_CHECK_EQ(icu_chain_token_number(chain), 15);

    icu_chain_destroy(chain);
}

static void check_chain_empty_chain(void)
{
    UErrorCode status = U_ZERO_ERROR;
    struct icu_chain *chain = 0;

    const char *xml_str = "<icu locale=\"en\">"
        "</icu>";

    const char *src8 = "some 5487 weired !¤%&(/& sTuFf";
    char *dest8 = 0;

    xmlDoc *doc = xmlParseMemory(xml_str, strlen(xml_str));
    xmlNode *xml_node = xmlDocGetRootElement(doc);
    YAZ_CHECK(xml_node);

    chain = icu_chain_xml_config(xml_node, 0, &status);

    xmlFreeDoc(doc);
    YAZ_CHECK(chain);

    YAZ_CHECK(icu_chain_assign_cstr(
                  chain,  src8,
                  &status));

    while (icu_chain_next_token(chain, &status))
    {
        ;
        /* printf("%d '%s' '%s'\n",
           icu_chain_token_number(chain),
           icu_chain_token_norm(chain),
           icu_chain_token_display(chain)); */
    }

    YAZ_CHECK_EQ(icu_chain_token_number(chain), 1);

    dest8 = (char *) icu_chain_token_norm(chain);
    YAZ_CHECK_EQ(strcmp(src8, dest8), 0);

    icu_chain_destroy(chain);
}

static void check_icu_iter1(void)
{
    UErrorCode status = U_ZERO_ERROR;
    struct icu_chain *chain = 0;
    xmlNode *xml_node;
    yaz_icu_iter_t iter;

    const char *xml_str = "<icu locale=\"en\">"
        "<tokenize rule=\"w\"/>"
        "<transform rule=\"[[:WhiteSpace:][:Punctuation:]] Remove\"/>"
        "</icu>";

    xmlDoc *doc = xmlParseMemory(xml_str, strlen(xml_str));
    YAZ_CHECK(doc);
    if (!doc)
        return;
    xml_node = xmlDocGetRootElement(doc);
    YAZ_CHECK(xml_node);
    if (!xml_node)
        return ;

    chain = icu_chain_xml_config(xml_node, 1, &status);

    xmlFreeDoc(doc);
    YAZ_CHECK(chain);

    iter = icu_iter_create(chain);
    icu_iter_first(iter, "a string with 15 tokens and 8 displays");
    YAZ_CHECK(iter);
    if (!iter)
        return;
    while (icu_iter_next(iter))
    {
        yaz_log(YLOG_LOG, "[%s]", icu_iter_get_norm(iter));
    }
    icu_iter_destroy(iter);
    icu_chain_destroy(chain);
}

static int test_iter(struct icu_chain *chain, const char *input,
                     const char *expected)
{
    yaz_icu_iter_t iter = icu_iter_create(chain);
    WRBUF result, second, sort_result;
    int success = 1;

    if (!iter)
    {
        yaz_log(YLOG_WARN, "test_iter: input=%s !iter", input);
        return 0;
    }

    if (icu_iter_next(iter))
    {
        yaz_log(YLOG_WARN, "test_iter: expecting 0 before icu_iter_first");
        return 0;
    }

    sort_result = wrbuf_alloc();
    result = wrbuf_alloc();
    icu_iter_first(iter, input);
    while (icu_iter_next(iter))
    {
        const char *sort_str = icu_iter_get_sortkey(iter);
        if (sort_str)
        {
            wrbuf_puts(sort_result, "[");
            wrbuf_puts_escaped(sort_result, sort_str);
            wrbuf_puts(sort_result, "]");
        }
        else
        {
            wrbuf_puts(sort_result, "[NULL]");
        }
        wrbuf_puts(result, "[");
        wrbuf_puts(result, icu_iter_get_norm(iter));
        wrbuf_puts(result, "]");
    }
    yaz_log(YLOG_LOG, "sortkey=%s", wrbuf_cstr(sort_result));
    second = wrbuf_alloc();
    icu_iter_first(iter, input);
    while (icu_iter_next(iter))
    {
        wrbuf_puts(second, "[");
        wrbuf_puts(second, icu_iter_get_norm(iter));
        wrbuf_puts(second, "]");
    }

    icu_iter_destroy(iter);

    if (strcmp(expected, wrbuf_cstr(result)))
    {
        yaz_log(YLOG_WARN, "test_iter: input=%s expected=%s got=%s",
                input, expected, wrbuf_cstr(result));
        success = 0;
    }

    if (strcmp(expected, wrbuf_cstr(second)))
    {
        yaz_log(YLOG_WARN, "test_iter: input=%s expected=%s got=%s (2nd)",
                input, expected, wrbuf_cstr(second));
        success = 0;
    }

    wrbuf_destroy(result);
    wrbuf_destroy(second);
    wrbuf_destroy(sort_result);
    return success;
}

static void *iter_thread(void *p)
{
    struct icu_chain *chain = (struct icu_chain *) p;
    int i;

    for (i = 0; i < 1000; i++)
    {
        YAZ_CHECK(test_iter(chain, "Adobe Acrobat Reader, 1991-1999.",
                            "[adobe][acrobat][reader][1991][][1999][]"));
    }
    return 0;
}

static void check_iter_threads(struct icu_chain *chain)
{
#if YAZ_POSIX_THREADS
#define NO_THREADS 1

    pthread_t t[NO_THREADS];
    int i;

    for (i = 0; i < NO_THREADS; i++)
        pthread_create(t + i, 0, iter_thread, chain);

    for (i = 0; i < NO_THREADS; i++)
        pthread_join(t[i], 0);
#endif
}

static void check_icu_iter2(void)
{
    UErrorCode status = U_ZERO_ERROR;
    struct icu_chain *chain = 0;
    xmlNode *xml_node;

    const char *xml_str = "<icu locale=\"en\">"
        "<transform rule=\"[:Control:] Any-Remove\"/>"
        "<tokenize rule=\"l\"/>"
        "<tokenize rule=\"w\"/>"
        "<transform rule=\"[[:WhiteSpace:][:Punctuation:]] Remove\"/>"
        "<display/>"
        "<casemap rule=\"l\"/>"
        "</icu>";

    xmlDoc *doc = xmlParseMemory(xml_str, strlen(xml_str));
    YAZ_CHECK(doc);
    if (!doc)
        return;
    xml_node = xmlDocGetRootElement(doc);
    YAZ_CHECK(xml_node);
    if (!xml_node)
        return ;

    chain = icu_chain_xml_config(xml_node, 1, &status);

    xmlFreeDoc(doc);
    YAZ_CHECK(chain);
    if (!chain)
        return;

    YAZ_CHECK(test_iter(chain, "Adobe Acrobat Reader, 1991-1999.",
                        "[adobe][acrobat][reader][1991][][1999][]"));

    YAZ_CHECK(test_iter(chain, "Νόταρης, Γιάννης Σωτ",
                        "[νόταρης][γιάννης][σωτ]"));

    check_iter_threads(chain);

    icu_chain_destroy(chain);
}

static void check_icu_iter3(void)
{
    UErrorCode status = U_ZERO_ERROR;
    struct icu_chain *chain = 0;
    xmlNode *xml_node;

    const char *xml_str =
        "<icu_chain id=\"sort\" locale=\"el\">\n"
        "<transform rule=\"[:Control:] Any-Remove\"/>\n"
        "<transform rule=\"[[:Control:][:WhiteSpace:][:Punctuation:]] Remove\"/>\n"
        "<transform rule=\"NFD; [:Nonspacing Mark:] Remove; NFC\"/>\n"
        "<casemap rule=\"l\"/>\n"
        "<display/>\n"
        "</icu_chain>\n";

    xmlDoc *doc = xmlParseMemory(xml_str, strlen(xml_str));
    YAZ_CHECK(doc);
    if (!doc)
        return;
    xml_node = xmlDocGetRootElement(doc);
    YAZ_CHECK(xml_node);
    if (!xml_node)
        return ;

    chain = icu_chain_xml_config(xml_node, 1, &status);

    xmlFreeDoc(doc);
    YAZ_CHECK(chain);
    if (!chain)
        return;

    YAZ_CHECK(test_iter(chain, "Adobe Acrobat Reader, 1991-1999.",
                        "[adobeacrobatreader19911999]"));

    YAZ_CHECK(test_iter(chain, "Νόταρης, Γιάννης Σωτ",
                        "[νοταρηςγιαννηςσωτ]"));

    icu_chain_destroy(chain);
}


static void check_icu_iter4(void)
{
    UErrorCode status = U_ZERO_ERROR;
    struct icu_chain *chain = 0;
    xmlNode *xml_node;

    const char *xml_str = "<icu locale=\"en\">"
        "<transform rule=\"[:Control:] Any-Remove\"/>"
        "<tokenize rule=\"l\"/>"
        "<tokenize rule=\"w\"/>"
        "<transform rule=\"[[:WhiteSpace:][:Punctuation:]] Remove\"/>"
        "<display/>"
        "<casemap rule=\"l\"/>"
        "<join rule=\"\"/>"
        "</icu>";

    xmlDoc *doc = xmlParseMemory(xml_str, strlen(xml_str));
    YAZ_CHECK(doc);
    if (!doc)
        return;
    xml_node = xmlDocGetRootElement(doc);
    YAZ_CHECK(xml_node);
    if (!xml_node)
        return ;

    chain = icu_chain_xml_config(xml_node, 1, &status);

    xmlFreeDoc(doc);
    YAZ_CHECK(chain);
    if (!chain)
        return;

    YAZ_CHECK(test_iter(chain, "Adobe Acrobat Reader, 1991-1999.",
                        "[adobeacrobatreader19911999]"));

    YAZ_CHECK(test_iter(chain, "Νόταρης, Γιάννης Σωτ",
                        "[νόταρηςγιάννηςσωτ]"));

    // check_iter_threads(chain);

    icu_chain_destroy(chain);
}


#endif /* YAZ_HAVE_ICU */

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();

#if YAZ_HAVE_ICU

    check_icu_casemap();
    check_icu_sortmap();
    check_icu_normalizer();
    check_icu_tokenizer();
    check_icu_chain();
    check_chain_empty_token();
    check_chain_empty_chain();
    check_icu_iter1();
    check_icu_iter2();
    check_icu_iter3();
    check_icu_iter4();

    check_bug_1140();

    u_cleanup();
#if YAZ_HAVE_XML2
    xmlCleanupParser();
#endif

#else /* YAZ_HAVE_ICU */

    yaz_log(YLOG_LOG, "ICU unit tests omitted");
    YAZ_CHECK(0 == 0);

#endif /* YAZ_HAVE_ICU */

    YAZ_CHECK_TERM;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

