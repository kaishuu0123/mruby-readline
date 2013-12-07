module Readline
  HISTORY = []
  class << self
    attr_accessor :completion_append_character
    attr_accessor :completion_proc
  end

  def self.readline(prompt = "", add_hist = false)
    line = Readline._c_readline(prompt, add_hist)

    if add_hist
      HISTORY.push(line)
    end

    return line
  end
end
