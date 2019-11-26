module CKB
  class Error < ::StandardError
  end

  class IndexOutOfBound < Error
  end

  class ItemMissing < Error
  end

  module Source
    INPUT = 1
    OUTPUT = 2
    CELL_DEP = 3
    HEADER_DEP = 4
    GROUP_INPUT = 0x0100000000000001
    GROPU_OUTPUT = 0x0100000000000002
  end

  class Reader
    def exists?
      !!length
    end

    def length
      internal_read(0)
    end

    def read(offset, len)
      internal_read(len, offset)
    end

    def readall
      # This way we can save one extra syscall
      l = length
      return nil unless l
      read(0, l)
    end
  end

  class Cell < Reader
    def initialize(source, index)
      @source = source
      @index = index
    end
  end

  class CellField < Reader
    CAPACITY = 0
    DATA_HASH = 1
    LOCK = 2
    LOCK_HASH = 3
    TYPE = 4
    TYPE_HASH = 5
    OCCUPIED_CAPACITY = 6

    def initialize(source, index, cell_field)
      @source = source
      @index = index
      @field = cell_field
    end
  end

  class CellData < Reader
    def initialize(source, index)
      @source = source
      @index = index
    end
  end

  class InputField < Reader
    OUT_POINT = 0
    SINCE = 1

    def initialize(source, index, input_field)
      @source = source
      @index = index
      @field = input_field
    end
  end

  class Header < Reader
    def initialize(source, index)
      @source = source
      @index = index
    end
  end

  class Witness < Reader
    def initialize(source, index)
      @source = source
      @index = index
    end
  end

  class Script < Reader
  end

  class Transaction < Reader
  end
end
