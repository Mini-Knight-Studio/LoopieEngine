#include "Clipboard.h"

namespace Loopie
{
	void Clipboard::Copy(std::string uuid)
	{
		m_uuid = uuid;
	}

	std::string Clipboard::Paste()
	{
		return m_uuid;
	}
}