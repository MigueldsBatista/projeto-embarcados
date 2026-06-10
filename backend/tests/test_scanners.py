import pytest
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from backend.database import Base
from backend.models import Scanner

# Setup test database
SQLALCHEMY_DATABASE_URL = "sqlite:///./test_scanners.db"
engine = create_engine(SQLALCHEMY_DATABASE_URL, connect_args={"check_same_thread": False})
TestingSessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)

@pytest.fixture(autouse=True)
def setup_db():
    Base.metadata.create_all(bind=engine)
    yield
    Base.metadata.drop_all(bind=engine)

def test_create_scanner():
    db = TestingSessionLocal()
    scanner = Scanner(identifier="esp32_01", name="Portão Principal")
    db.add(scanner)
    db.commit()
    db.refresh(scanner)
    assert scanner.id is not None
    assert scanner.identifier == "esp32_01"
    db.close()
