cdef class PyTrain:
    @staticmethod
    def Init(np.ndarray[double, ndim=1, mode="c"] _feNum not None,
        np.ndarray[int, ndim=1, mode="c"]  _facCard not None,
        _cardMax,
        _nPredNum,
        _nPredFac,
        _nRow,
        _nTree,
        _nSamp,
        np.ndarray[double, ndim=1, mode="c"] _feSampleWeight not None,
        withRepl,
        _trainBlock,
        _minNode,
        _minRatio,
        _totLevels,
        _ctgWidth,
        _predFixed,
        np.ndarray[double, ndim=1, mode="c"] _predProb not None,
        np.ndarray[double, ndim=1, mode="c"] _regMono not None):
        return Train_Init(&_feNum[0],
            &_facCard[0],
            _cardMax,
            _nPredNum,
            _nPredFac,
            _nRow,
            _nTree,
            _nSamp,
            &_feSampleWeight[0],
            withRepl,
            _trainBlock,
            _minNode,
            _minRatio,
            _totLevels,
            _ctgWidth,
            _predFixed,
            &_predProb[0],
            &_regMono[0])

    @staticmethod
    def Regression(np.ndarray[int, ndim=1, mode="c"] _feRow not None,
        np.ndarray[int, ndim=1, mode="c"]  _feRank not None,
        np.ndarray[int, ndim=1, mode="c"]  _feInvNum not None,
        _y,
        _row2Rank,
        _origin,
        _facOrigin,
        np.ndarray[double, ndim=1, mode="c"] _predInfo not None,
        vector[ForestNode] &_forestNode,
        _facSplit,
        _leafOrigin,
        vector[LeafNode] &_leafNode,
        vector[BagRow] &_bagRow,
        _rank):
        return Train_Regression(&_feRow[0],
            &_feRank[0],
            &_feInvNum[0],
            _y,
            _row2Rank,
            _origin,
            _facOrigin,
            &_predInfo[0],
            vector[ForestNode] &_forestNode,
            _facSplit,
            _leafOrigin,
            vector[LeafNode] &_leafNode,
            vector[BagRow] &_bagRow,
            _rank)

    @staticmethod
    def Classification(np.ndarray[int, ndim=1, mode="c"]  _feRow not None,
        np.ndarray[int, ndim=1, mode="c"]  _feRank not None,
        np.ndarray[int, ndim=1, mode="c"]  _feInvNum not None,
        _yCtg,
        _ctgWidth,
        _yProxy,
        _origin,
        _facOrigin,
        np.ndarray[double, ndim=1, mode="c"] _predInfo not None,
        vector[ForestNode] &_forestNode,
        _facSplit,
        _leafOrigin,
        vector[LeafNode] &_leafNode,
        vector[BagRow] &_bagRow,
        _weight):
        return Train_Classification(&_feRow[0],
            &_feRank[0],
            &_feInvNum[0],
            _yCtg,
            _ctgWidth,
            _yProxy,
            _origin,
            _facOrigin,
            &_predInfo[0],
            vector[ForestNode] &_forestNode,
            _facSplit,
            _leafOrigin,
            vector[LeafNode] &_leafNode,
            vector[BagRow] &_bagRow,
            _weight)
