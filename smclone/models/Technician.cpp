#include "Technician.h"

Technician::Technician(int id, const QString &name, bool active)
    : m_id(id), m_name(name), m_active(active)
{
}

int Technician::id() const { return m_id; }
void Technician::setId(int id) { m_id = id; }

QString Technician::name() const { return m_name; }
void Technician::setName(const QString &name) { m_name = name; }

bool Technician::active() const { return m_active; }
void Technician::setActive(bool active) { m_active = active; }
