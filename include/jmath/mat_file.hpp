#ifndef JMATH_MAT_FILE_HPP
#define JMATH_MAT_FILE_HPP

#include "boost/numeric/ublas/vector.hpp"
#include "boost/numeric/ublas/matrix.hpp"

#include "kernel/csvFile.hpp"
#include "jmath/jmathException.hpp"

namespace jafar {

	namespace jmath {
		/**Class matrix_file loads/saves data into/from a matrix from/to a file.
		 * @remark matrix is publicly accessed through data member 
		 * @ingroup jmath
		 * see a demo in jmath/demo_suite/mat_file.cpp
		 */
		template<typename T>
		class matrix_file : public jafar::kernel::CSVFileSaveLoad {
		public:
			boost::numeric::ublas::matrix<T> data;
			///default constructor
      matrix_file() : jafar::kernel::CSVFileSaveLoad(), data(0,0) {}
			///constructor which initializes data content to @param _data
			matrix_file(const boost::numeric::ublas::matrix<T> &_data) : 
				jafar::kernel::CSVFileSaveLoad(), data(_data) {}
			protected:
			void loadCSVFile(jafar::kernel::CSVFile& csvFile) {
				data.resize(csvFile.nbOfLines(), csvFile.nbOfColumns());
				JFR_PRED_ERROR((data.size1() == csvFile.nbOfLines()) &&
											 (data.size2() == csvFile.nbOfColumns()),
											 JmathException,
											 JmathException::RESIZE_FAIL,
											 "resizing data to "<<csvFile.nbOfLines()
											 <<"x"<<csvFile.nbOfColumns()
											 <<" failed, abort")
				for(unsigned int row = 0; row < csvFile.nbOfLines(); row++)
					for(unsigned int col = 0; col < csvFile.nbOfColumns(); col++)
						csvFile.getItem(row, col, data(row,col));
			}
			void saveCSVFile(jafar::kernel::CSVFile& csvFile) {
				size_t rows = data.size1();
				size_t cols = data.size2();
				for(unsigned int row = 0; row < rows; row++)
					for(unsigned int col = 0; col < cols; col++)
						csvFile.setItem(row, col, data(row,col));
			}
		};

		/**Class vector_file loads/saves data into/from a vector from/to a file.
		 * @remark vector is publicly accessed through data member 
		 * @remark assumes data written in a single column or single row
		 * @ingroup jmath
		 */
		template<typename T>
		class vector_file : public jafar::kernel::CSVFileSaveLoad {
		public:
			boost::numeric::ublas::vector<T> data;
			typedef enum {VERTICAL, HORIZONTAL} storage;
			storage storage_type;
			///default constructor
      vector_file(storage type = VERTICAL) : 
				jafar::kernel::CSVFileSaveLoad(), data(0), storage_type(type) {}
			///constructor which initializes data content to @param _data
			vector_file(const boost::numeric::ublas::vector<T>& _data, storage type = VERTICAL) : 
				jafar::kernel::CSVFileSaveLoad(), data(_data), storage_type(type) {}
			protected:
			void loadCSVFile(jafar::kernel::CSVFile& csvFile) {
				JFR_PRED_ERROR((csvFile.nbOfLines() == 1) || (csvFile.nbOfColumns() == 1),
											 JmathException,
											 JmathException::WRONG_SIZE,
											 "data should be on single column or line")
				size_t length = std::max(csvFile.nbOfLines(), csvFile.nbOfColumns());
				data.resize(length);
				JFR_PRED_ERROR(data.size() == length,
											 JmathException,
											 JmathException::RESIZE_FAIL,
											 "resizing data to "<<length<<" failed, abort")
					if(length == csvFile.nbOfLines())
						for(unsigned int row = 0; row < length; row++)
							csvFile.getItem(row, 0, data[row]);
					else
						for(unsigned int col = 0; col < length; col++)
							csvFile.getItem(0, col, data[col]);
			}
			void saveCSVFile(jafar::kernel::CSVFile& csvFile) {
				size_t length = data.size();
				switch(storage_type){
				case VERTICAL : {
					for(unsigned int row = 0; row < length; row++)
						csvFile.setItem(row, 0, data[row]);
				}; break;
				case HORIZONTAL : {
					for(unsigned int col = 0; col < length; col++)
						csvFile.setItem(0, col, data[col]);
				}; break;
				default : {
					JFR_ERROR(JmathException,
										JmathException::WRONG_TYPE,
										"can not handle this kind of storage direction");
				}
				}
			}
		};
	}
}

#endif
