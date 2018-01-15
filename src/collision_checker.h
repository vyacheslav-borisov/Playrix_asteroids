#pragma once

#include "cell_grid.h"

using namespace math;

//Класс CollisionManager предназначен для выявления столкновений или пересечений игровых объектов
//С каждым игровым объектом можно связать один или несколько ограничивающих контуров (CollisionHull), 
//обладающих позицией и размерами.
//CollisionManager отслеживает положения таких контуров в каждом цикле и составляет список пересекаюшихся пар.
//Каждый контур принадлежит к определенной группе столкновений (collisionGroup). Если объекты принадлежат одной группе
//проверка на их столкновения не проводиться.
namespace pegas
{
	//интерфейс ограничивающего контура.
	//здесь реализовано три типа ограничивающих контуров:
	//точка, окружность и полигон
	//в игровом коде не надо создавать эти объекты напрямую, это делает
	//CollisionManager
	class ICollisionHull : public RefCounter
	{
	public:
		enum
		{
			k_typePoint = 0,
			k_typeCircle,
			k_typePolygon,
			k_typeTotal
		};
	public:
		ICollisionHull(int id, int collisionGroup):
		  m_id(id), m_collisionGroup(collisionGroup) {}
		virtual ~ICollisionHull() {}

		int getId() const { return m_id; }
		int getCollisionGroup() const { return m_collisionGroup; }
		virtual int getType() = 0;

		virtual void moveObject(const Vector3& offset, bool absolute) = 0;
		virtual void rotateObject(float degreesOffset, bool absolute) = 0;
		virtual void transformObject(const Matrix4& m) = 0;

		virtual Vector3 getPosition() = 0;
		virtual void draw() = 0;

	protected:
		int m_id;
		int m_collisionGroup;
	};

	class CollisionManager
	{
	public:
		typedef std::vector<Vector3> PointList;
		typedef std::pair<int, int> CollisionPair;
		typedef std::list<CollisionPair> CollisionPairList;
		typedef CollisionPairList::iterator CollisionPairListIt;
		
		typedef boost::intrusive_ptr<ICollisionHull> CollisionHullPtr;
		typedef std::map<int, CollisionHullPtr> CollisionHullMap;
		typedef std::set<int> CollisionPairsHashes;

	public:
		CollisionManager();
		~CollisionManager();

		//зарегистрировать контур
		//id - заданный пользователем идентификатор, по которому он будет отслеживаться
		//любое уникальное целое число, например адрес в памяти игрового объекта
		//group - коллизионная группа. Проверка на столкновения производиться только между контурами
		//из разных групп.
		bool registerPoint(int id, int group, const Vector3& position);
		bool registerCircle(int id, int group, const Vector3& position, float radius);
		bool registerPoligon(int id, int group, const PointList& points);
		void unregisterCollisionHull(int id);
		
		//обновить положение контура
		//absolute - если задано true, позиция задаеться относительно начального положения
		//заданного при регистрации контура, false - относительно текущего положения
		void moveObject(int id, const Vector3& offset, bool absolute = true);
		void rotateObject(int id, float degreesOffset, bool absolute = true);
		void transformObject(int id, const Matrix4& m);
		
		//обновления списка коллизий, метод нужно вызывать в каждом цикле игры
		void update();
		//список колизий в данном цикле
		CollisionPairList& getCollidedPairs();

		bool isIntersects(ICollisionHull* a, ICollisionHull* b);

		//визуализация контуров на экране, нужна в целях отладки столкновений
		void debugDraw();

	private:
		static bool isIntersectsPointCircle(ICollisionHull* point, ICollisionHull* circle);
		static bool isIntersectsCirclePoint(ICollisionHull* circle, ICollisionHull* point);

		static bool isIntersectsPointPolygon(ICollisionHull* point, ICollisionHull* polygon);
		static bool isIntersectsPolygonPoint(ICollisionHull* polygon, ICollisionHull* point);

		static bool isIntersectsCirclePolygon(ICollisionHull* circle, ICollisionHull* polygon);
		static bool isIntersectsPolygonCircle(ICollisionHull* polygon, ICollisionHull* circle);
		
		static bool isIntersectsPointPoint(ICollisionHull* point1, ICollisionHull* point2);
		static bool isIntersectsCircleCircle(ICollisionHull* circle1, ICollisionHull* circle2);
		static bool isIntersectsPolygonPolygon(ICollisionHull* polygon1, ICollisionHull* polygon2);

	private:
		CellGrid<ICollisionHull*> m_cellGrid;
		CollisionHullMap m_collisionHulls;
		CollisionPairsHashes m_previousCollisionPairs;
		CollisionPairList m_pairs;

		typedef bool (*IntersectsChecker)(ICollisionHull* a, ICollisionHull* b);
		IntersectsChecker m_checkers[ICollisionHull::k_typeTotal][ICollisionHull::k_typeTotal];
	};

	//колизионный контур - точка
	//хорошо годиться для "воплощения" пуль и снарядов
	class PointCollisionHull: public ICollisionHull 
	{
	public:
		PointCollisionHull(int id, int group, const Vector3& position);

		virtual int getType() { return k_typePoint; }

		virtual void moveObject(const Vector3& offset, bool absolute);
		virtual void rotateObject(float degreesOffset, bool absolute);
		virtual void transformObject(const Matrix4& m);
		virtual Vector3 getPosition();
		virtual void draw() { }

	protected:
		Vector3 m_initialPosition;
		Vector3 m_currentPosition;
	};

	//окружность для воплощения объектов обладающих некоторыми размерами
	//например персонажей. разумный компромис между точностью и скоростью проверки
	class CircleCollisionHull: public PointCollisionHull
	{
	public:
		CircleCollisionHull(int id, int group, const Vector3& position, float radius);

		virtual int getType() { return k_typeCircle; }
		float getRadius() const { return m_radius; }
		
		virtual void draw();

	protected:
		float m_radius;
	};

	//выпуклый многоугольник
	//для объектов сложной формы, которым нужна точная проверка на столкновения
	//например статические препятствия или стены в лабиринте
	class PoligonCollisionHull: public ICollisionHull
	{
	public:
		PoligonCollisionHull(int id, int group, const CollisionManager::PointList& points);

		virtual int getType() { return k_typePolygon; }

		virtual void moveObject(const Vector3& offset, bool absolute);
		virtual void rotateObject(float degreesOffset, bool absolute);
		virtual void transformObject(const Matrix4& m);

		virtual Vector3 getPosition();
		CollisionManager::PointList getPoints() const { return m_currentPoints; }

		virtual void draw();

	protected:
		CollisionManager::PointList m_initalPoints;
		CollisionManager::PointList m_currentPoints;
		Vector3 m_initialPosition;
		Vector3 m_currentPosition;
	};
}


