/*
 * libopenraw - xmlhandler.cpp
 *
 * Copyright (C) 2008-2015 Hubert Figuiere
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <utility>

#include <libxml/xmlreader.h>

#include "xmlhandler.h"

namespace xml {

bool LtString::operator()(const xmlChar *s1, const xmlChar *s2) const {
    return strcmp((const char *)s1, (const char *)s2) < 0;
}

Context::Context(const HandlerPtr &handler) : m_handler(handler) {
}

Context::~Context() {
}

ContextPtr Context::startElement(const int32_t /*element*/) {
    return shared_from_this();
}

void Context::endElement(const int32_t /*element*/) {
}

void Context::appendText(const xmlChar * /*content*/) {
}

Handler::Handler(const std::string &filename)
    : Context(HandlerPtr())
    , m_reader(xmlNewTextReaderFilename(filename.c_str())) {
}

Handler::~Handler() {
    if (m_reader != NULL) {
        xmlFreeTextReader(m_reader);
    }
}

void Handler::mapTags(const tag_map_definition_t *map) {
    m_tag_map.clear();
    const tag_map_definition_t *ptag = map;
    while (ptag->first != 0) {
        m_tag_map.insert(
            std::make_pair((const xmlChar *)ptag->first, ptag->second));
        ptag++;
    }
}

int32_t Handler::getTagId(const xmlChar *tag) {
    if (tag == NULL) {
        return 0;
    }
    tag_map_t::const_iterator iter = m_tag_map.find(tag);
    if (iter == m_tag_map.end()) {
        fprintf(stderr, "Tag %s is unknown\n", tag);
        return 0;
    }
    return iter->second;
}

bool Handler::process() {
    if (m_reader == NULL) {
        return false;
    }
    m_contexts.push(shared_from_this());

    int ret = xmlTextReaderRead(m_reader);
    while (ret == 1) {
        int node_type = xmlTextReaderNodeType(m_reader);
        switch (node_type) {
        case XML_READER_TYPE_ELEMENT: {
            int32_t element = getTagId(xmlTextReaderConstName(m_reader));
            ContextPtr context = m_contexts.top()->startElement(element);
            m_contexts.push(context);
            break;
        }
        case XML_READER_TYPE_TEXT: {
            const xmlChar *content = xmlTextReaderConstValue(m_reader);
            m_contexts.top()->appendText(content);
            break;
        }
        case XML_READER_TYPE_END_ELEMENT: {
            int32_t element = getTagId(xmlTextReaderConstName(m_reader));
            m_contexts.top()->endElement(element);
            m_contexts.pop();
            break;
        }
        default:
            break;
        }
        ret = xmlTextReaderRead(m_reader);
    }
    // make sure we clear the contexts.
    while (!m_contexts.empty()) {
        m_contexts.pop();
    }
    return true;
}

SimpleElementContext::SimpleElementContext(const HandlerPtr &handler,
                                           std::string &content)
    : Context(handler), m_content(content) {
}

void SimpleElementContext::appendText(const xmlChar *content) {
    m_content += (const char *)content;
}
}
