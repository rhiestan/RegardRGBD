#ifndef VECTOR_H
#define VECTOR_H

/**
 * This template is a simple N-dimensional vector.
 *
 * The data members are not initialized to 0!
 */
template <typename T, unsigned int C>
class SimpleVector
{
public:

	SimpleVector() { }

	SimpleVector(const SimpleVector &other)
	{
		for(unsigned int i = 0; i < C; i++)
		{
			c[i] = other.c[i];
		}
	}

	SimpleVector(T v1, T v2)
	{
		c[0] = v1;
		c[1] = v2;
	}

	SimpleVector(T v1, T v2, T v3)
	{
		c[0] = v1;
		c[1] = v2;
		c[2] = v3;
	}

	T &operator [](unsigned int i)
	{
		return c[i];
	}

	const T &operator [](unsigned int i) const
	{
		return c[i];
	}

	SimpleVector &operator=(const SimpleVector & other)
	{
		for(unsigned int i = 0; i < C; i++)
		{
			c[i] = other.c[i];
		}
		return *this;
	}

	// The components (still public to make access easier and faster)
	T c[C];
};

typedef SimpleVector<float, 3> SVector3f;
typedef SimpleVector<double, 3> SVector3d;
typedef SimpleVector<unsigned char, 3> SVector3b;

#endif
