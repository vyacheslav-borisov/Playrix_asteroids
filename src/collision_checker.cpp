#include "stdafx.h"
#include "collision_checker.h"

using namespace math;

namespace pegas
{
	Vector3 operator*(const Vector3& a, const Matrix4& b)
	{
		Vector3 result;

		result.x = a.x * b._11 + a.y * b._21 + a.z * b._31 + b._41;
		result.y = a.x * b._12 + a.y * b._22 + a.z * b._32 + b._42;
		result.z = a.x * b._13 + a.y * b._23 + a.z * b._33 + b._43;

		return result;
	}
	//--------------------------------------------------------------------------------------------------------
	//	CollisionManager implementation
	//--------------------------------------------------------------------------------------------------------
	CollisionManager::CollisionManager()
	{
		m_cellGrid.create(10000, 10000, 50);

		m_checkers[ICollisionHull::k_typePoint][ICollisionHull::k_typePoint] = &CollisionManager::isIntersectsPointPoint;
		m_checkers[ICollisionHull::k_typePoint][ICollisionHull::k_typeCircle] = &CollisionManager::isIntersectsPointCircle;
		m_checkers[ICollisionHull::k_typePoint][ICollisionHull::k_typePolygon] = &CollisionManager::isIntersectsPointPolygon;

		m_checkers[ICollisionHull::k_typeCircle][ICollisionHull::k_typePoint] = &CollisionManager::isIntersectsCirclePoint;
		m_checkers[ICollisionHull::k_typeCircle][ICollisionHull::k_typeCircle] = &CollisionManager::isIntersectsCircleCircle;
		m_checkers[ICollisionHull::k_typeCircle][ICollisionHull::k_typePolygon] = &CollisionManager::isIntersectsCirclePolygon;

		m_checkers[ICollisionHull::k_typePolygon][ICollisionHull::k_typePoint] = &CollisionManager::isIntersectsPolygonPoint;
		m_checkers[ICollisionHull::k_typePolygon][ICollisionHull::k_typeCircle] = &CollisionManager::isIntersectsPolygonCircle;
		m_checkers[ICollisionHull::k_typePolygon][ICollisionHull::k_typePolygon] = &CollisionManager::isIntersectsPolygonPolygon;
	}

	CollisionManager::~CollisionManager()
	{
		m_cellGrid.destroy();
	}

	bool CollisionManager::isIntersects(ICollisionHull* a, ICollisionHull* b)
	{
		int aIndex = a->getType();
		int bIndex = b->getType();

		return (m_checkers[aIndex][bIndex])(a, b);
	}

	bool CollisionManager::registerPoint(int id, int group, const Vector3& position)
	{
		assert(id > 0);
		assert(group > 0);
		assert(m_collisionHulls.count(id) == 0);

		if(m_collisionHulls.count(id) > 0)
		{
			return false;
		}

		CollisionHullPtr hull = new PointCollisionHull(id, group, position);
		m_collisionHulls[id] = hull;
		m_cellGrid.placeToGrid(hull->getPosition(), hull.get());

		return true;
	}
	
	bool CollisionManager::registerCircle(int id, int group, const Vector3& position, float radius)
	{
		assert(id > 0);
		assert(group > 0);
		assert(m_collisionHulls.count(id) == 0);

		if(m_collisionHulls.count(id) > 0)
		{
			return false;
		}
		
		
		CollisionHullPtr hull = new CircleCollisionHull(id, group, position, radius);
		m_collisionHulls[id] = hull;
		m_cellGrid.placeToGrid(hull->getPosition(), hull.get());
		
		return true;
	}
	
	bool CollisionManager::registerPoligon(int id, int group, const PointList& points)
	{
		assert(id > 0);
		assert(group > 0);
		assert(m_collisionHulls.count(id) == 0);

		if(m_collisionHulls.count(id) > 0)
		{
			return false;
		}

		
		CollisionHullPtr hull = new PoligonCollisionHull(id, group, points);
		m_collisionHulls[id] = hull;
		m_cellGrid.placeToGrid(hull->getPosition(), hull.get());

		return true;
	}
	
	void CollisionManager::unregisterCollisionHull(int id)
	{
		//assert(m_collisionHulls.count(id) > 0);
		
		if(m_collisionHulls.count(id) > 0)
		{
			CollisionHullPtr hull = m_collisionHulls[id];
			m_collisionHulls.erase(id);
			m_cellGrid.removeObject(hull.get());
		}
	}
		
	void CollisionManager::moveObject(int id, const Vector3& offset, bool absolute)
	{
		assert(m_collisionHulls.count(id) > 0);

		CollisionHullPtr hull = m_collisionHulls[id];
		hull->moveObject(offset, absolute);
		m_cellGrid.placeToGrid(hull->getPosition(), hull.get());
	}
	
	void CollisionManager::rotateObject(int id, float degreesOffset, bool absolute)
	{
		assert(m_collisionHulls.count(id) > 0);

		CollisionHullPtr hull = m_collisionHulls[id];
		hull->rotateObject(degreesOffset, absolute);
		m_cellGrid.placeToGrid(hull->getPosition(), hull.get());
	}

	void CollisionManager::transformObject(int id, const Matrix4& m)
	{
		assert(m_collisionHulls.count(id) > 0);

		CollisionHullPtr hull = m_collisionHulls[id];
		hull->transformObject(m);
		m_cellGrid.placeToGrid(hull->getPosition(), hull.get());
	}
		
	void CollisionManager::update()
	{
		std::set<int> closedNodes;

		m_pairs.clear();

		for(CollisionHullMap::iterator it = m_collisionHulls.begin(); it != m_collisionHulls.end(); ++it)
		{
			Vector3 position = ((it->second).get())->getPosition();
			Cell<ICollisionHull*>* myCell =	m_cellGrid.getCell(position); 

			ICollisionHull* a = it->second.get();
			ICollisionHull* b = 0;

			for(int i = 0; i < 9; i++)
			{
				Cell<ICollisionHull*>* currentCell = myCell->getSibling(i);
				if(currentCell == 0) continue;
				
				for(Cell<ICollisionHull*>::ObjectListIt iit = currentCell->begin(); iit != currentCell->end(); ++iit)
				{
					b = *iit;

					if(a == b) continue;
					if(closedNodes.count(b->getId()) > 0) continue;

					//TODO: collision groups filter
					if(a->getCollisionGroup() == b->getCollisionGroup()) continue;

					int id_a = a->getId();
					int id_b = b->getId();
					int hash = std::max(id_a, id_b) << 16 | std::min(id_a, id_b);

					if(isIntersects(a, b))
					{
						if(m_previousCollisionPairs.count(hash) > 0)
						{
							continue;
						}
						
						CollisionPair pair(std::max(id_a, id_b), std::min(id_a, id_b));
						m_pairs.push_back(pair);
						m_previousCollisionPairs.insert(hash);
					}else
					{
						m_previousCollisionPairs.erase(hash);
					}
					
				}//for(Cell<ICollisionHull*>::ObjectListIt iit = currentCell->begin(); iit != currentCell->end(); ++iit)
			}//for(int i = 0; i < 9; i++)

			closedNodes.insert(a->getId());

		}//for(CollisionHullMap::iterator it = m_collisionHulls.begin(); it != m_collisionHulls.end(); ++it)
	}
	
	CollisionManager::CollisionPairList& CollisionManager::getCollidedPairs()
	{
		return m_pairs;
	}

	void CollisionManager::debugDraw()
	{
		for(CollisionHullMap::iterator it = m_collisionHulls.begin(); it != m_collisionHulls.end(); ++it)
		{
			((it->second).get())->draw();
		}
	}


	//-----------------------------------------------------------------------------------------------
	//	Collision checkers
	//-----------------------------------------------------------------------------------------------
	bool CollisionManager::isIntersectsPointCircle(ICollisionHull* point, ICollisionHull* circle)
	{
		PointCollisionHull* pointCH = dynamic_cast<PointCollisionHull*>(point);
		CircleCollisionHull* circleCH = dynamic_cast<CircleCollisionHull*>(circle);

		Vector3 dv = pointCH->getPosition() - circleCH->getPosition();
		float distance = dv.Length();

		return (distance <= circleCH->getRadius());
	}

	bool CollisionManager::isIntersectsCirclePoint(ICollisionHull* circle, ICollisionHull* point)
	{
		return isIntersectsPointCircle(point, circle);
	}

	bool CollisionManager::isIntersectsPointPolygon(ICollisionHull* point, ICollisionHull* polygon)
	{
		PointCollisionHull* pointCH = dynamic_cast<PointCollisionHull*>(point);
		PoligonCollisionHull* poligonCH = dynamic_cast<PoligonCollisionHull*>(polygon);

		Vector3 position = pointCH->getPosition();
		PointList points = poligonCH->getPoints();

		int i0, i1;
		float A, B, C, D;
		Vector3 P0, P1;
		for(size_t i = 0; i < points.size(); i++)
		{
			i0 = i;
			i1 = (i == (points.size() - 1)) ? 0 : i + 1;
			
			P0 = points[i0];
			P1 = points[i1];
			
			A = P0.y - P1.y;
			B = P1.x - P0.x;
			C = (P0.x * P1.y) - (P1.x * P0.y);
			D =  (A * position.x) + (B * position.y) + C;

			if(D > 0)
			{
				return false;
			}
		}

		return true;
	}

	bool CollisionManager::isIntersectsPolygonPoint(ICollisionHull* polygon, ICollisionHull* point)
	{
		return isIntersectsPointPolygon(point, polygon);
	}

	bool CollisionManager::isIntersectsCirclePolygon(ICollisionHull* circle, ICollisionHull* polygon)
	{
		CircleCollisionHull* circleCH = dynamic_cast<CircleCollisionHull*>(circle);
		PoligonCollisionHull* poligonCH = dynamic_cast<PoligonCollisionHull*>(polygon);

		Vector3 position = circleCH->getPosition();
		float radius = circleCH->getRadius();
		PointList points = poligonCH->getPoints();
		
		Vector3 dv;
		float distance;
		for(size_t i = 0; i < points.size(); i++)
		{
			dv = position - points[i];
			distance = dv.Length();
			if(distance <= radius)
			{
				return true;
			}
		}

		return false;
	}

	bool CollisionManager::isIntersectsPolygonCircle(ICollisionHull* polygon, ICollisionHull* circle)
	{
		return isIntersectsCirclePolygon(circle, polygon);
	}
		
	bool CollisionManager::isIntersectsPointPoint(ICollisionHull* point1, ICollisionHull* point2)
	{
		PointCollisionHull* pointCH1 = dynamic_cast<PointCollisionHull*>(point1);
		PointCollisionHull* pointCH2 = dynamic_cast<PointCollisionHull*>(point2);
		
		Vector3 p1 = pointCH1->getPosition();
		Vector3 p2 = pointCH2->getPosition();
		
		const float epsilon = 0.001;
		bool b1 = std::abs(p1.x - p2.x) < epsilon;
		bool b2 = std::abs(p1.y - p2.y) < epsilon;

		return (b1 && b2);
	}

	bool CollisionManager::isIntersectsCircleCircle(ICollisionHull* circle1, ICollisionHull* circle2)
	{
		CircleCollisionHull* circleCH1 = dynamic_cast<CircleCollisionHull*>(circle1);
		CircleCollisionHull* circleCH2 = dynamic_cast<CircleCollisionHull*>(circle2);

		Vector3 p1 = circleCH1->getPosition();
		Vector3 p2 = circleCH2->getPosition();
		float r1 = circleCH1->getRadius();
		float r2 = circleCH2->getRadius();
		
		Vector3 dv = p1 - p2;
		float distance = dv.Length();

		return (distance < (r1 + r2));
	}

	bool CollisionManager::isIntersectsPolygonPolygon(ICollisionHull* polygon1, ICollisionHull* polygon2)
	{
		PoligonCollisionHull* poligonCH1 = dynamic_cast<PoligonCollisionHull*>(polygon1);
		PoligonCollisionHull* poligonCH2 = dynamic_cast<PoligonCollisionHull*>(polygon2);

		PointList points1 = poligonCH1->getPoints();
		PointList points2 = poligonCH2->getPoints();

		std::vector<float> A1(points1.size());
		std::vector<float> B1(points1.size());
		std::vector<float> C1(points1.size());

		std::vector<float> A2(points2.size());
		std::vector<float> B2(points2.size());
		std::vector<float> C2(points2.size());

		int i0, i1;
		float D;
		Vector3 P0, P1;
		for(size_t i = 0; i < points1.size(); i++)
		{
			i0 = i;
			i1 = (i == (points1.size() - 1)) ? 0 : i + 1;
			
			P0 = points1[i0];
			P1 = points1[i1];
			
			A1[i] = P0.y - P1.y;
			B1[i] = P1.x - P0.x;
			C1[i] = (P0.x * P1.y) - (P1.x * P0.y);
		}

		for(size_t i = 0; i < points2.size(); i++)
		{
			i0 = i;
			i1 = (i == (points2.size() - 1)) ? 0 : i + 1;
			
			P0 = points2[i0];
			P1 = points2[i1];
			
			A2[i] = P0.y - P1.y;
			B2[i] = P1.x - P0.x;
			C2[i] = (P0.x * P1.y) - (P1.x * P0.y);
		}

		//cheking 1 against 2
		for(size_t i = 0; i < points1.size(); i++)
		{
			for(size_t j = 0; j < points2.size(); j++)
			{
				P0 = points1[i];
				D = (P0.x * A2[j]) + (P0.y * B2[j]) + C2[j];
				if(D > 0)
				{
					return true;
				}
			}
		}//cheking 1 against 2

		//cheking 2 against 1
		for(size_t i = 0; i < points2.size(); i++)
		{
			for(size_t j = 0; j < points1.size(); j++)
			{
				P0 = points2[i];
				D = (P0.x * A1[j]) + (P0.y * B1[j]) + C1[j];
				if(D > 0)
				{
					return true;
				}
			}
		}//cheking 2 against 1

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//	PointCollisionHull class implementation
	//------------------------------------------------------------------------------------------------
	PointCollisionHull::PointCollisionHull(int id, int group, const Vector3& position)
		:ICollisionHull(id, group), m_initialPosition(position), m_currentPosition(position)
	{

	}

	void PointCollisionHull::moveObject(const Vector3& offset, bool absolute)
	{
		m_currentPosition = absolute ? (m_initialPosition + offset) : (m_currentPosition + offset); 
	}

	void PointCollisionHull::rotateObject(float degreesOffset, bool absolute)
	{
		Matrix4 mat = Matrix4::RotationZ(degreesOffset);
		
		m_currentPosition = absolute ? (m_initialPosition * mat) : (m_currentPosition * mat);
	}

	void PointCollisionHull::transformObject(const Matrix4& m)
	{
		m_currentPosition = m_initialPosition * m;
	}

	Vector3 PointCollisionHull::getPosition()
	{
		return m_currentPosition;
	}

	//------------------------------------------------------------------------------------------------
	//	CircleCollisionHull class imlementation
	//-------------------------------------------------------------------------------------------------
	CircleCollisionHull::CircleCollisionHull(int id, int group, const Vector3& position, float radius)
		:PointCollisionHull(id, group, position), m_radius(radius)
	{
		
	}

	void CircleCollisionHull::draw()
	{
		//TODO: CircleCollisionHull::draw
		/*
		CURCOORD left, top, width, height;
		RGBCOLOR color = 0xffff0000;

		width = height = m_radius * 2;
		left = m_currentPosition.x - m_radius;
		top = m_currentPosition.y - m_radius;

		graph.drawEllipse(left, top, width, height, color, 0x00000000);*/
	}

	//---------------------------------------------------------------------------------------------------
	//	PoligonCollisionHull class implementation
	//---------------------------------------------------------------------------------------------------
	PoligonCollisionHull::PoligonCollisionHull(int id, int group, const CollisionManager::PointList& points)
		:ICollisionHull(id, group), m_initalPoints(points.begin(), points.end()), m_currentPoints(points.begin(), points.end())
	{
		for(size_t i = 0; i < m_currentPoints.size(); i++)
		{
			m_currentPosition = m_currentPosition + m_currentPoints[i];
		}
		m_currentPosition = m_currentPosition / m_currentPoints.size();
		m_initialPosition = m_currentPosition;
	}

	void PoligonCollisionHull::moveObject(const Vector3& offset, bool absolute)
	{
		for(size_t i = 0; i < m_currentPoints.size(); i++)
		{
			m_currentPoints[i] = absolute ? (m_initalPoints[i] + offset) : (m_currentPoints[i] + offset); 
		}

		m_currentPosition = absolute ? (m_initialPosition + offset) : (m_currentPosition + offset);
	}

	void PoligonCollisionHull::rotateObject(float degreesOffset, bool absolute)
	{
		Matrix4 mat = Matrix4::RotationZ(degreesOffset);
		
		for(size_t i = 0; i < m_currentPoints.size(); i++)
		{
			m_currentPoints[i] = absolute ? (m_initalPoints[i] * mat) : (m_currentPoints[i] * mat); 
		}

		m_currentPosition = absolute ? (m_initialPosition * mat) : (m_currentPosition * mat);
	}

	void PoligonCollisionHull::transformObject(const Matrix4& m)
	{
		for(size_t i = 0; i < m_currentPoints.size(); i++)
		{
			m_currentPoints[i] = m_initalPoints[i] * m; 
		}

		m_currentPosition = m_initialPosition * m;
	}

	Vector3 PoligonCollisionHull::getPosition()
	{
		return m_currentPosition; 
	}

	void PoligonCollisionHull::draw()
	{
		//TODO: PoligonCollisionHull::draw
		/*
		CURCOORD fromX, fromY, toX, toY;
		RGBCOLOR color = 0xffff0000;

		for(int i = 1; i < m_currentPoints.size(); i++)
		{
			fromX = m_currentPoints[i - 1].x;
			fromY = m_currentPoints[i - 1].y;
			toX = m_currentPoints[i].x;
			toY = m_currentPoints[i].y;

			graph.drawLine(fromX, fromY, toX, toY, color);
		}

		int iLast = m_currentPoints.size() - 1;

		fromX = m_currentPoints[0].x;
		fromY = m_currentPoints[0].y;
		toX = m_currentPoints[iLast].x;
		toY = m_currentPoints[iLast].y;

		graph.drawLine(fromX, fromY, toX, toY, color);*/
	}
}


