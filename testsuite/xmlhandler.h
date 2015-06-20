/* -*- Mode: C++ -*- */
/*
 * libopenraw - xmlhandler.h
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


#ifndef _XML_HANDLER_H_
#define _XML_HANDLER_H_

#include <stdint.h>

#include <libxml/xmlstring.h>
#include <libxml/xmlreader.h>

#include <stack>
#include <string>
#include <map>
#include <memory>

namespace xml {


class LtString
{
public:
	bool operator()(const xmlChar* s1, const xmlChar* s2) const;
};


class Handler;
typedef std::shared_ptr<Handler> HandlerPtr;

class Context;
typedef std::shared_ptr<Context> ContextPtr;

class Context
	: public std::enable_shared_from_this<Context>
{
public:
	Context(const HandlerPtr & handler);
	virtual ~Context();

	virtual ContextPtr startElement(int32_t element);
	virtual void endElement(int32_t element);
	virtual void appendText(const xmlChar * content);
protected:
	HandlerPtr m_handler;
};



struct tag_map_definition_t;

class Handler
	: public Context
{
public:
	typedef std::map<const xmlChar *, int32_t, xml::LtString> tag_map_t;

	Handler(const std::string & filename);
	~Handler();

	int32_t getTagId(const xmlChar * tag);
	bool process();

protected:
	void mapTags(const tag_map_definition_t * map);

private:
	std::stack<ContextPtr> m_contexts;
	tag_map_t              m_tag_map;
	xmlTextReaderPtr       m_reader;
};


struct tag_map_definition_t
{
	const char * first;
	int32_t second;
};


class SimpleElementContext
	: public Context
{
public:
	SimpleElementContext(const HandlerPtr & handler, std::string & content);
	virtual void appendText(const xmlChar * content) override;
private:
	std::string & m_content;
};

}


#endif
