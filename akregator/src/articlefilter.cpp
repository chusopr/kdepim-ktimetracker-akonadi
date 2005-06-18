/*
 * articlefilter.cpp
 *
 * Copyright (c) 2004, 2005 Frerich Raabe <raabe@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "articlefilter.h"
#include "article.h"
#include "tag.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kurl.h>

#include <qregexp.h>

namespace Akregator {

Criterion::Criterion()
{
}

Criterion::Criterion( Subject subject, Predicate predicate, const QVariant &object )
    : m_subject( subject )
    , m_predicate( predicate )
    , m_object( object )
{
}

bool Criterion::satisfiedBy( const Article &article ) const
{
    QVariant concreteSubject;

    switch ( m_subject ) {
        case Title:
            concreteSubject = QVariant(article.title());
            break;
        case Description:
            concreteSubject = QVariant(article.description());
            break;
        case Link:
            // ### Maybe use prettyURL here?
            concreteSubject = QVariant(article.link().url());
            break;
        case Status:
            concreteSubject = QVariant(article.status());
            break;
        case KeepFlag:
            concreteSubject = QVariant(article.keep());   
        default:
            break;
    }

    bool satisfied = false;

    const Predicate predicateType = static_cast<Predicate>( m_predicate & ~Negation );
	QString subjectType=concreteSubject.typeName();

    switch ( predicateType ) {
        case Contains:
            satisfied = concreteSubject.toString().find( m_object.toString(), 0, false ) != -1;
            break;
        case Equals:
            if (subjectType=="int")
                satisfied = concreteSubject.toInt() == m_object.toInt();
            else
                satisfied = concreteSubject.toString() == m_object.toString();
            break;
        case Matches:
            satisfied = QRegExp( m_object.toString() ).search( concreteSubject.toString() ) != -1;
            break;
        default:
            kdDebug() << "Internal inconsistency; predicateType should never be Negation" << endl;
            break;
    }

    if ( m_predicate & Negation ) {
        satisfied = !satisfied;
    }

    return satisfied;
}

Criterion::Subject Criterion::subject() const
{
    return m_subject;
}

Criterion::Predicate Criterion::predicate() const
{
    return m_predicate;
}

QVariant Criterion::object() const
{
    return m_object;
}

ArticleFilter::ArticleFilter()
    : m_association( None )
    , m_action( NoAction )
{
}

ArticleFilter::ArticleFilter( const QValueList<Criterion> &criteria, Association assoc, Action action )
    : m_criteria( criteria )
    , m_association( assoc )
    , m_action( action )
{
}

bool ArticleFilter::matches( const Article &a ) const
{
    switch ( m_association ) {
        case LogicalOr:
            return anyCriterionMatches( a );
        case LogicalAnd:
            return allCriteriaMatch( a );
        default:
            break;
    }
    return true;
}

ArticleFilter::Action ArticleFilter::action() const
{
    return m_action;
}

bool ArticleFilter::operator==(const AbstractFilter& other) const
{
    AbstractFilter* ptr = const_cast<AbstractFilter*>(&other);
    ArticleFilter* o = dynamic_cast<ArticleFilter*>(ptr);
    if (!o)
        return false;
    else
        return m_action == o->m_action && m_association == o->m_association && m_criteria == o->m_criteria;
}
bool ArticleFilter::operator!=(const AbstractFilter& other) const
{
    return !(*this == other);
}

bool ArticleFilter::anyCriterionMatches( const Article &a ) const
{
    if (m_criteria.count()==0)
        return true;
    QValueList<Criterion>::ConstIterator it = m_criteria.begin();
    QValueList<Criterion>::ConstIterator end = m_criteria.end();
    for ( ; it != end; ++it ) {
        if ( ( *it ).satisfiedBy( a ) ) {
            return true;
        }
    }
    return false;
}

bool ArticleFilter::allCriteriaMatch( const Article &a ) const
{
    if (m_criteria.count()==0)
        return true;
    QValueList<Criterion>::ConstIterator it = m_criteria.begin();
    QValueList<Criterion>::ConstIterator end = m_criteria.end();
    for ( ; it != end; ++it ) {
        if ( !( *it ).satisfiedBy( a ) ) {
            return false;
        }
    }
    return true;
}

class TagFilter::TagFilterPrivate
{
    public:
    Tag tag;
    bool operator==(const TagFilterPrivate& other) const
    {
        return tag == other.tag;
    }
};

TagFilter::TagFilter(const Tag& tag) : d(new TagFilterPrivate)
{
    d->tag = tag;
}

TagFilter::TagFilter() : d(new TagFilterPrivate)
{
}

TagFilter::~TagFilter()
{
    delete d;
    d = 0;
}

bool TagFilter::matches(const Article& article) const
{
    return article.hasTag(d->tag.id());
}


TagFilter::TagFilter(const TagFilter& other) : AbstractFilter(other), d(0)
{
    *this = other;
}

bool TagFilter::operator==(const AbstractFilter& other) const
{
    AbstractFilter* ptr = const_cast<AbstractFilter*>(&other);
    TagFilter* tagFilter = dynamic_cast<TagFilter*>(ptr);
    return tagFilter ? *d == *(tagFilter->d) : false;
}

bool TagFilter::operator!=(const AbstractFilter &other) const
{
    return !(*this == other);
}

TagFilter& TagFilter::operator=(const TagFilter& other)
{
    d = new TagFilterPrivate;
    *d = *(other.d);
    return *this;
}

} //namespace Akregator
