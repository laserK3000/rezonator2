#include "Format.h"
#include "Protocol.h"
#include "Schema.h"
#include "Utils.h"

//------------------------------------------------------------------------------
//                               SchemaListener
//------------------------------------------------------------------------------

SchemaListener::~SchemaListener()
{
}

//------------------------------------------------------------------------------
//                                SchemaState
//------------------------------------------------------------------------------

QString SchemaState::str() const
{
    switch (_current)
    {
    case None: return QStringLiteral("SchemaState: None");
    case New: return QStringLiteral("SchemaState: New");
    case Loading: return QStringLiteral("SchemaState: Loading");
    case Modified: return QStringLiteral("SchemaState: Modified");
    }
    return QString();
}

void SchemaState::set(State state)
{
    _current = state;
}

//------------------------------------------------------------------------------
//                                SchemaEvents
//------------------------------------------------------------------------------

#define INIT_EVENT(event, raise_changed, next_state)\
    {event, EventProps{QString(# event), raise_changed, static_cast<SchemaState::State>(next_state)}}

void SchemaEvents::raise(Event event, void *param, const char* reason) const
{
    if (!_enabled) return;

    QString alias = _schema->alias();
    if (!alias.isEmpty())
        alias = QStringLiteral("[%1]: ").arg(alias);

    const EventProps& eventProps = propsOf(event);
    Z_REPORT(QStringLiteral("%1SchemaEvent: %2, reason=[%3]").arg(alias).arg(eventProps.name).arg(reason))

    if (int(eventProps.nextState) != SchemaState::Current)
    {
        _schema->state().set(eventProps.nextState);
        Z_REPORT(QStringLiteral("%1%2").arg(alias).arg(_schema->state().str()))
    }

    auto listeners = _schema->listeners();

    for (SchemaListener* listener : listeners)
        notify(listener, event, param);

    if (eventProps.shouldRaiseChanged)
    {
        Z_REPORT(QStringLiteral("%1SchemaEvent: %2, modified=%3, reason=[%4]")
            .arg(alias).arg(propsOf(Changed).name).arg(Z::str(_schema->modified())).arg(reason))
        for (SchemaListener* listener : listeners)
            notify(listener, Changed, nullptr);
    }
}

const SchemaEvents::EventProps& SchemaEvents::propsOf(Event event)
{
    static QMap<Event, EventProps> _props(
    {
        //                           | Should also | New state which
        //                           | raise event | schema obtains
        //                           | 'Changed'   | with this event
        INIT_EVENT(Created,            false,        SchemaState::New      ),
        INIT_EVENT(Deleted,            false,        SchemaState::Current  ),
        INIT_EVENT(Changed,            false,        SchemaState::Modified ),
        INIT_EVENT(Saved,              true,         SchemaState::None     ),
        INIT_EVENT(Loading,            false,        SchemaState::Loading  ),
        INIT_EVENT(Loaded,             true,         SchemaState::None     ),
        INIT_EVENT(Rebuilt,            true,         SchemaState::Modified ),
        INIT_EVENT(ElemCreated,        true,         SchemaState::Modified ),
        INIT_EVENT(ElemChanged,        true,         SchemaState::Modified ),
        INIT_EVENT(ElemDeleting,       false,        SchemaState::Current  ),
        INIT_EVENT(ElemDeleted,        true,         SchemaState::Modified ),
        INIT_EVENT(ParamsChanged,      true,         SchemaState::Modified ),
        INIT_EVENT(LambdaChanged,      true,         SchemaState::Modified ),
        INIT_EVENT(CustomParamCreated, true,         SchemaState::Modified ),
        INIT_EVENT(CustomParamEdited,  true,         SchemaState::Modified ),
        INIT_EVENT(CustomParamChanged, true,         SchemaState::Modified ),
        INIT_EVENT(CustomParamDeleted, true,         SchemaState::Modified ),
        INIT_EVENT(CustomParamDeleting,true,         SchemaState::Current  ),
        INIT_EVENT(PumpCreated,        true,         SchemaState::Modified ),
        INIT_EVENT(PumpChanged,        true,         SchemaState::Modified ),
        INIT_EVENT(PumpDeleted,        true,         SchemaState::Modified ),
        INIT_EVENT(PumpDeleting,       false,        SchemaState::Current  ),
        INIT_EVENT(RecalRequred,       false,        SchemaState::Current  ),
    });
    return _props[event];
}

void SchemaEvents::notify(SchemaListener* listener, SchemaEvents::Event event, void *param) const
{
    switch (event)
    {
    case Created: listener->schemaCreated(_schema); break;
    case Deleted: listener->schemaDeleted(_schema); break;
    case Changed: listener->schemaChanged(_schema); break;
    case Saved: listener->schemaSaved(_schema); break;
    case Loading: listener->schemaLoading(_schema); break;
    case Loaded: listener->schemaLoaded(_schema); break;
    case Rebuilt: listener->schemaRebuilt(_schema); break;
    case ElemCreated: listener->elementCreated(_schema, reinterpret_cast<Element*>(param)); break;
    case ElemChanged: listener->elementChanged(_schema, reinterpret_cast<Element*>(param)); break;
    case ElemDeleting: listener->elementDeleting(_schema, reinterpret_cast<Element*>(param)); break;
    case ElemDeleted: listener->elementDeleted(_schema, reinterpret_cast<Element*>(param)); break;
    case ParamsChanged: listener->schemaParamsChanged(_schema); break;
    case LambdaChanged: listener->schemaLambdaChanged(_schema); break;
    case CustomParamCreated: listener->customParamCreated(_schema, reinterpret_cast<Z::Parameter*>(param)); break;
    case CustomParamEdited: listener->customParamEdited(_schema, reinterpret_cast<Z::Parameter*>(param)); break;
    case CustomParamChanged: listener->customParamChanged(_schema, reinterpret_cast<Z::Parameter*>(param)); break;
    case CustomParamDeleting: listener->customParamDeleting(_schema, reinterpret_cast<Z::Parameter*>(param)); break;
    case CustomParamDeleted: listener->customParamDeleted(_schema, reinterpret_cast<Z::Parameter*>(param)); break;
    case PumpCreated: listener->pumpCreated(_schema, reinterpret_cast<PumpParams*>(param)); break;
    case PumpChanged: listener->pumpChanged(_schema, reinterpret_cast<PumpParams*>(param)); break;
    case PumpDeleting: listener->pumpDeleting(_schema, reinterpret_cast<PumpParams*>(param)); break;
    case PumpDeleted: listener->pumpDeleted(_schema, reinterpret_cast<PumpParams*>(param)); break;
    case RecalRequred: listener->recalcRequired(_schema); break;
    }
}

//------------------------------------------------------------------------------
//                                 ElementSelector
//------------------------------------------------------------------------------

ElementSelector::~ElementSelector()
{
}

//------------------------------------------------------------------------------
//                                 Schema
//------------------------------------------------------------------------------

Schema::Schema(const QString &alias) : _alias(alias)
{
    _wavelength = Z::Parameter(Z::Dims::linear(), "lambda", Z::Strs::lambda(), /*qApp->translate("Param", "Wavelength")*/QString());
    _wavelength.setValue(Z::Value(980, Z::Units::nm()));
    _wavelength.addListener(this);

    _events._schema = this;
    _events.raise(SchemaEvents::Created, "Schema: schema constructor");
}

Schema::~Schema()
{
    _events.raise(SchemaEvents::Deleted, "schema: schema destructor");

    qDeleteAll(_items);
    qDeleteAll(_customParams);
    qDeleteAll(_pumps);

    _formulas.clear();
}

int Schema::enabledCount() const
{
    int count = 0;
    for (int i= 0; i < _items.size(); i++)
        if (!_items.at(i)->disabled())
            count++;
    return count;
}

Element* Schema::element(int index) const
{
    return isValid(index)? _items.at(index): nullptr;
}

Element* Schema::elementById(int id) const
{
    foreach (Element *elem, _items)
        if (elem->id() == id)
            return elem;
    return nullptr;
}

Element* Schema::elementByLabel(const QString& label) const
{
    foreach (Element *elem, _items)
        if (elem->label() == label)
            return elem;
    return nullptr;
}

void Schema::insertElement(Element* elem, int index, bool event)
{
    if (!elem) return;

    if (isValid(index))
        _items.insert(index, elem);
    else
        _items.append(elem);

    elem->setOwner(this);

    relinkInterfaces();

    if (event)
    {
        _events.raise(SchemaEvents::ElemCreated, elem, "Schema: insertElement");
        _events.raise(SchemaEvents::RecalRequred, "Schema: insertElement");
    }
}

void Schema::insertElements(const Elements& elems, int index, bool event)
{
    int insert = isValid(index);
    for (int i = 0; i < elems.size(); i++)
    {
        auto elem = elems.at(i);

        if (insert)
            _items.insert(index + i, elem);
        else
            _items.append(elem);

        elem->setOwner(this);
    }

    relinkInterfaces();

    if (event)
    {
        for (auto elem : elems)
            _events.raise(SchemaEvents::ElemCreated, elem, "Schema: insertElements");
        _events.raise(SchemaEvents::RecalRequred, "Schema: insertElements");
    }
}

void Schema::deleteElement(Element* elem, bool event, bool free)
{
    deleteElement(_items.indexOf(elem), event, free);
}

void Schema::deleteElement(int index, bool event, bool free)
{
    if (!isValid(index)) return;

    Element *elem = _items.at(index);

    if (event)
        _events.raise(SchemaEvents::ElemDeleting, elem, "Schema: deleteElement");

    _items.removeAt(index);
    elem->setOwner(nullptr);

    relinkInterfaces();
    removeParamLinks(elem);

    if (event)
    {
        _events.raise(SchemaEvents::ElemDeleted, elem, "Schema: deleteElement");
        _events.raise(SchemaEvents::RecalRequred, "Schema: deleteElement");
    }

    if (free)
        delete elem;
}

void Schema::clearElements(bool events)
{
    while (_items.size() > 0)
    {
        Element *elem = _items.at(0);

        if (events)
            _events.raise(SchemaEvents::ElemDeleting, elem, "Schema: clearElements");

        _items.removeAt(0);
        elem->setOwner(nullptr);

        if (events)
            _events.raise(SchemaEvents::ElemDeleted, elem, "Schema: clearElements");

        delete elem;
    }

    qDeleteAll(_paramLinks);

    if (events)
        _events.raise(SchemaEvents::RecalRequred, "Schema: clearElements");
}

void Schema::elementChanged(Element *elem)
{
    _events.raise(SchemaEvents::ElemChanged, elem, "Schema: elementChanged");
}

void Schema::parameterChanged(Z::ParameterBase *param)
{
    if (param == &_wavelength)
    {
        _events.raise(SchemaEvents::LambdaChanged, "Schema: parameterChanged: lambda");
        _events.raise(SchemaEvents::RecalRequred, "Schema: parameterChanged: lambda");
    }
}

void Schema::setTripType(TripType value)
{
    if (_tripType == value) return;
    _tripType = value;
    _events.raise(SchemaEvents::ParamsChanged, "Schema: setTripType");
    _events.raise(SchemaEvents::RecalRequred, "Schema: setTripType");
}

Z::Parameters Schema::globalParams() const
{
    Z::Parameters list(_customParams);
    list << const_cast<Z::Parameter*>(&_wavelength);
    return list;
}

PumpParams* Schema::activePump()
{
    for (PumpParams *pump : _pumps)
        if (pump->isActive())
            return pump;
    return nullptr;
}

void Schema::removeParamLinks(Element* elem)
{
    for (auto param: elem->params())
    {
        while (auto link = _paramLinks.byTarget(param))
        {
            _paramLinks.removeOne(link);
            delete link;
        }
        while (auto link = _paramLinks.bySource(param))
        {
            _paramLinks.removeOne(link);
            delete link;
        }
    }
}

void Schema::relinkInterfaces()
{
    int elemCount = _items.size();
    for (int i = 0; i < elemCount; i++)
    {
        ElementInterface *iface = dynamic_cast<ElementInterface*>(_items.at(i));
        if (!iface) continue;

        ElementLocker locker(iface);

        removeParamLinks(iface);

        ElementRange *left = nullptr;
        ElementRange *right = nullptr;

        if (i == 0)
        {
            if (tripType() == TripType::RR)
                left = dynamic_cast<ElementRange*>(_items.at(elemCount-1));
        }
        else left = dynamic_cast<ElementRange*>(_items.at(i-1));
        if (left)
        {
            auto link = new Z::ParamLink(left->paramIor(), iface->paramIor1());
            link->setOption(Z::ParamLink_NonStorable);
            _paramLinks.append(link);
        }
        else
            iface->paramIor1()->setValue(1);

        if (i == elemCount - 1)
        {
            if (tripType() == TripType::RR)
                right = dynamic_cast<ElementRange*>(_items.at(0));
        }
        else right = dynamic_cast<ElementRange*>(_items.at(i+1));
        if (right)
        {
            auto link = new Z::ParamLink(right->paramIor(), iface->paramIor2());
            link->setOption(Z::ParamLink_NonStorable);
            _paramLinks.append(link);
        }
        else
            iface->paramIor2()->setValue(1);
    }
}

void Schema::shiftElement(int index, const std::function<int(int)>& getTargetIndex)
{
    if (!isValid(index)) return;
    if (_items.size() == 1) return;
    _items.swap(index, getTargetIndex(index));
    relinkInterfaces();
    _events.raise(SchemaEvents::Rebuilt, "Schema: shiftElement");
    _events.raise(SchemaEvents::RecalRequred, "Schema: shiftElement");
}

void Schema::moveElementUp(Element *elem)
{
    shiftElement(indexOf(elem), [&](int index){
        return index == 0 ? _items.size() - 1 : index - 1;
    });
}

void Schema::moveElementDown(Element *elem)
{
    shiftElement(indexOf(elem), [&](int index){
        return index == _items.size() - 1 ? 0 : index + 1;
    });
}

void Schema::flip()
{
    int size = _items.size();
    if (size < 2) return;
    for (int i = 0; i < size / 2; i++)
        _items.swap(i, size - 1 - i);
    relinkInterfaces();
    _events.raise(SchemaEvents::Rebuilt, "Schema: flip");
    _events.raise(SchemaEvents::RecalRequred, "Schema: flip");
}

void Schema::markModified(const char *reason)
{
    _events.raise(SchemaEvents::Changed, reason);
}

void Schema::calcMatrices(const char* reason)
{
    qDebug() << "Schema::calcMatrices" << alias() << reason;
    for (auto elem : _items)
        elem->calcMatrix("Schema::calcMatrices");
}

//------------------------------------------------------------------------------
//                                Z::Utils
//------------------------------------------------------------------------------

namespace Z {
namespace Utils {

void generateLabel(const Elements& elements, Element* elem, const QString &labelPrefix)
{
    QStringList labels;
    for (const auto e : elements)
        if (e != elem)
            labels << e->label();
    QString prefix = labelPrefix.isEmpty() ? elem->labelPrefix() : labelPrefix;
    elem->setLabel(generateLabel(prefix, labels));
}

void generateLabel(Schema* schema, PumpParams* pump)
{
    QStringList labels;
    for (const auto p : *schema->pumps())
        if (p != pump)
            labels << p->label();
    pump->setLabel(generateLabel(Pump::labelPrefix(), labels));
}

Element* findElemByParam(Schema* schema, Parameter* param)
{
    for (auto e : schema->elements())
        for (auto p : e->params())
            if (p == param)
                return e;
    return nullptr;
}

} // namespace Utils
} // namespace Z
