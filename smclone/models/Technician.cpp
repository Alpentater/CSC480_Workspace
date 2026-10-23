//Icluding the header file for the technician class.
#include "Technician.h"

//Technician constructor.
Technician::Technician(int id, const QString &name, bool active)
    : m_id(id), m_name(name), m_active(active)
{
}

//Getter and setter for technician ids.
int Technician::id() const { return m_id; }
void Technician::setId(int id) { m_id = id; }

//Getter and setter for technician names.
QString Technician::name() const { return m_name; }
void Technician::setName(const QString &name) { m_name = name; }


//Returns whether the technician is currently active.
//Active technicians are the ones that should normallyappear as columns on the scheduling screen.
//Below is a getter and setter functions for handling that value.
bool Technician::active() const { return m_active; }
void Technician::setActive(bool active) { m_active = active; }
