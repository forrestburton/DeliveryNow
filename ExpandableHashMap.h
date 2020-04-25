// ExpandableHashMap.h

// Implementation for an expandable hash map

const int STARTING_BUCKETS = 8;

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
	ExpandableHashMap(double maximumLoadFactor = 0.5);
	~ExpandableHashMap();
	void reset();
	int size() const;
	void associate(const KeyType& key, const ValueType& value);

	// for a map that can't be modified, return a pointer to const ValueType
	const ValueType* find(const KeyType& key) const;

	// for a modifiable map, return a pointer to modifiable ValueType
	ValueType* find(const KeyType& key)
	{
		return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
	}

	//Prevent copying and assignment
	ExpandableHashMap(const ExpandableHashMap&) = delete;
	ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
	void clearHash();
	int getBucket(const KeyType& key, const int& capacity) const;
	void expandHash();

	struct Node {
		KeyType key;
		ValueType value;
		Node* next = nullptr;
	};

	Node** m_hashMap;
	int m_maxSize;        //number of associations cannot exceed this
	int m_capacity;       //total number of buckets/size of array
	int m_size;           //number of associations currently in map
	double m_maxLoadFactor;
};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor)
	:m_capacity(STARTING_BUCKETS), m_size(0)
{
	if (maximumLoadFactor < 0)
		maximumLoadFactor = 0.5;
	m_maxLoadFactor = maximumLoadFactor;

	m_hashMap = new Node * [STARTING_BUCKETS];
	for (int i = 0; i < STARTING_BUCKETS; i++)  
		m_hashMap[i] = nullptr;

	m_maxSize = (int)(m_maxLoadFactor * STARTING_BUCKETS);
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
	clearHash();
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
	clearHash();

	m_capacity = STARTING_BUCKETS;
	m_maxSize = (int)(m_maxLoadFactor * STARTING_BUCKETS);
	m_hashMap = new Node * [STARTING_BUCKETS];
	for (int i = 0; i < STARTING_BUCKETS; i++)  
		m_hashMap[i] = nullptr;

	m_size = 0;           
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
	return m_size;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
	int bucketNum = getBucket(key, m_capacity);
	if (find(key) == nullptr) 
	{
		if (m_size + 1 > m_maxSize)  
			expandHash();
		if (m_hashMap[bucketNum] == nullptr)   
		{
			Node* insert = new Node;
			insert->key = key;
			insert->value = value;
			insert->next = nullptr;
			m_hashMap[bucketNum] = insert;
		}
		else  
		{
			Node* curr = m_hashMap[bucketNum];
			while (curr->next != nullptr)   
				curr = curr->next;
			Node* insert = new Node;
			insert->key = key;
			insert->value = value;
			insert->next = nullptr;
			curr->next = insert;
		}
		m_size++;
	}
	else
	{
		ValueType* val = find(key);
		*val = value;
	}
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::expandHash()
{
	int newCapacity = m_capacity * 2;
	Node** newMap = new Node * [newCapacity];
	for (int i = 0; i < newCapacity; i++)  
		newMap[i] = nullptr;

	for (int i = 0; i < m_capacity; i++)   
	{
		Node* curr = m_hashMap[i];
		while (curr != nullptr)   
		{
			KeyType copyKey = curr->key;  
			unsigned int bucketNum = getBucket(copyKey, newCapacity);  

			if (newMap[bucketNum] == nullptr)  
			{
				Node* insert = new Node;
				insert->key = copyKey;
				insert->value = curr->value;
				insert->next = nullptr;
				newMap[bucketNum] = insert;
			}
			else if (newMap[bucketNum]->key == curr->key)
				newMap[bucketNum]->value = curr->value;
			else 
			{
				Node* p = newMap[bucketNum];
				while (p->next != nullptr)   
					p = p->next;
				Node* insert = new Node;
				insert->key = copyKey;
				insert->value = curr->value;
				insert->next = nullptr;
				p->next = insert;
			}
			curr = curr->next;
		}
	}

	clearHash();
	m_hashMap = newMap;
	m_capacity = newCapacity;
	m_maxSize = (int)(m_maxLoadFactor * newCapacity);
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::getBucket(const KeyType& key, const int& capacity) const
{
	unsigned int hasher(const KeyType & key2);
	unsigned int bucketNum = hasher(key);
	return bucketNum % capacity;
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
	int bucketNum = getBucket(key, m_capacity);
	if (m_hashMap[bucketNum] == nullptr)  
		return nullptr;

	for (Node* curr = m_hashMap[bucketNum]; curr != nullptr; curr = curr->next) 
	{
		if (curr->key == key)
			return &(curr->value);    
	}
	return nullptr;  
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::clearHash()
{
	for (int i = 0; i < m_capacity; i++)   
	{
		Node* curr = m_hashMap[i];
		while (curr != nullptr)
		{
			Node* kill = curr;
			curr = curr->next;
			delete kill;
		}
	}
	delete[] m_hashMap;
}