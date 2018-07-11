/**
* Orion2 - A MapleStory2 Dynamic Link Library Localhost
*
* @author Eric
*
*/
#ifdef ORION_EXPORTS
#define ORION_API __declspec(dllexport)
#else
#define ORION_API __declspec(dllimport)
#endif

// Solely used as a dummy export for the Orion.dll
class ORION_API APIDummy {
public:
	APIDummy(void);
};
